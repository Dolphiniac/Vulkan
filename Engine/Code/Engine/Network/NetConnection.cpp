#include "Engine/Network/NetConnection.hpp"
#include "Engine/Network/NetPacket.hpp"
#include "Engine/Network/NetSession.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Profiler.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "../Core/EngineSystemManager.hpp"


//-----------------------------------------------------------------------------------------------
STATIC const float NetConnection::TIME_UNTIL_HEARTBEAT = 1.5f;
STATIC const float NetConnection::TIME_UNTIL_MARKED_BAD = 5.f;
STATIC const float NetConnection::TIME_UNTIL_DISCONNECTED = 15.f;


//-----------------------------------------------------------------------------------------------
NetConnection::NetConnection(byte index, const std::string& guid, const sockaddr_in& addr)
	: m_index(index)
	, m_guid(guid)
	, m_timeSinceLastReceivedPacket(0.f)
	, m_currentZerothAckBundleIndex(0)
	, m_highestReceivedAck(INVALID_PACKET_ACK)
	, m_currentAckIndex(0)
	, m_previousReceivedAckBitfield(0)
	, m_currentReliableID(0)
	, m_currentSequenceID(0)
	, m_nextExpectedReliableID(0)
	, m_nextExpectedSequenceID(0)
	, m_timeSinceLastHeartbeatSent(0.f)
	, m_timeSinceLastSentPacket(0.f)
{
	memcpy(&m_toAddr, &addr, sizeof(sockaddr_in));
}


//-----------------------------------------------------------------------------------------------
void NetConnection::PushReliablesIntoPacket(NetPacket& packet, PacketHeader* header)
{
	AckBundle bundle;
	bundle.ackID = m_currentAckIndex;

	bool pushedReliables = false;

	std::deque<NetMessage> workingReliablesQueue;
	std::swap(workingReliablesQueue, m_unconfirmedReliables);

	while (!workingReliablesQueue.empty())
	{
		//Work through unconfirmed reliables, adding them to the packet if possible
		//Work them back into the queue to be removed on ack
		NetMessage& message = workingReliablesQueue.back();
		bool success = packet.WriteContents(message);
		if (success)
		{
			pushedReliables = true;
			bundle.reliableIDs.push_back(message.GetReliableID());
			m_unconfirmedReliables.push_front(message);
			workingReliablesQueue.pop_back();
		}
		else
		{
			break;
		}
	}
	while (!workingReliablesQueue.empty())
	{
		//Clean up any reliables we couldn't write.  They'll go first in the queue
		NetMessage& toCopy = workingReliablesQueue.front();
		m_unconfirmedReliables.push_back(toCopy);
		workingReliablesQueue.pop_front();
	}

	if (pushedReliables)
	{
		ushort ackIndex = GetBundleIndexForID(m_currentAckIndex);
		AckBundle& thisBundle = m_ackBundles[ackIndex];
		thisBundle.reliableIDs.clear();
		thisBundle.reliableIDs.shrink_to_fit();
		thisBundle = bundle;
		header->thisAck = m_currentAckIndex;
		m_currentAckIndex++;
	}
	else
	{
		header->thisAck = INVALID_PACKET_ACK;
	}

	workingReliablesQueue.clear();
	workingReliablesQueue.shrink_to_fit();
}


//-----------------------------------------------------------------------------------------------
void NetConnection::PushUnreliablesIntoPacket(NetPacket& packet)
{
	while (!m_unsentUnreliables.empty())
	{
		NetMessage& message = m_unsentUnreliables.back();
		bool success = packet.WriteContents(message);
		if (success)
		{
			m_unsentUnreliables.pop_back();
		}
		else
		{
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
ushort NetConnection::GetBundleIndexForID(ushort ackIndex)
{
	while (ackIndex >= m_currentZerothAckBundleIndex + ARRAY_LENGTH(m_ackBundles))
	{
		m_currentZerothAckBundleIndex += ARRAY_LENGTH(m_ackBundles);
	}

	return ackIndex - m_currentZerothAckBundleIndex;
}


//-----------------------------------------------------------------------------------------------
bool NetConnection::ConstructPacketAndSend(byte playerIndex)
{
	if (m_unsentUnreliables.empty() && m_unconfirmedReliables.empty())
	{
		return false;
	}

	NetPacket packet;
	PacketHeader* header = packet.GetPacketHeader();
	header->playerIndex = playerIndex;
	header->mostRecentReceivedAck = m_highestReceivedAck;
	header->previousReceivedAckBitfield = m_previousReceivedAckBitfield;

	//Based on this implementation, the first reliable that doesn't fit in the packet won't get through
	//Consequently, it's possible that some unreliables will be preferred based on this criterion
	//I don't think it's a huge issue, though
	PushReliablesIntoPacket(packet, header);
	PushUnreliablesIntoPacket(packet);
	if (g_netSession)
	{
		m_timeSinceLastSentPacket = 0.f;
		g_netSession->SendPacketDirect(&m_toAddr, packet);
	}

	return !m_unsentUnreliables.empty();
}


//-----------------------------------------------------------------------------------------------
void NetConnection::AddMessage(NetMessage& message)
{
	if (message.IsOrdered())
	{
		ASSERT_OR_DIE(message.IsReliable(), "Cannot support unreliable ordered traffic");
		message.SetSequenceID(m_currentSequenceID++);
	}
	if (message.IsReliable())
	{
		message.SetReliableID(m_currentReliableID++);
		m_unconfirmedReliables.push_front(message);
	}
	else
	{
		m_unsentUnreliables.push_front(message);
	}
}


//-----------------------------------------------------------------------------------------------
bool NetConnection::IsMe() const
{
	if (!g_netSession)
	{
		return false;
	}

	return this == g_netSession->m_myConnection;
}


//-----------------------------------------------------------------------------------------------
static ushort GetAckIDForBitfieldIndex(int bitIndex, ushort mostRecentReceivedAck)
{
	return (ushort)(mostRecentReceivedAck - ((BITS_PER_ACK_FIELD - 1) - bitIndex));
}


//-----------------------------------------------------------------------------------------------
void NetConnection::UpdateAcksAndStatus(const NetPacket& packet)
{
	if (m_type == NETCONNECTIONTYPE_BAD || m_type == NETCONNECTIONTYPE_UNCONFIRMED)
	{
		m_type = NETCONNECTIONTYPE_CONFIRMED;
	}

	m_timeSinceLastReceivedPacket = 0.f;

	PacketHeader* header = packet.GetPacketHeader();

	if (header->thisAck != INVALID_PACKET_ACK)
	{
		//Update current receieved ack fields based on packet's ackID
		if (header->thisAck > m_highestReceivedAck)
		{
			ushort distToShift = header->thisAck - m_highestReceivedAck;
			m_highestReceivedAck = header->thisAck;
			m_previousReceivedAckBitfield >>= distToShift;
		}
		else if (m_highestReceivedAck == INVALID_PACKET_ACK)
		{
			m_highestReceivedAck = 0;
		}
		ushort difference = m_highestReceivedAck - header->thisAck;

		if (difference < BITS_PER_ACK_FIELD)
		{
			ushort distToShift = (BITS_PER_ACK_FIELD - 1) - difference;
			m_previousReceivedAckBitfield |= (1 << distToShift);
		}
	}

	//Now, for the bitfield the packet sent us, we update our unconfirmed reliables
	for (int bitIndex = 0; bitIndex < BITS_PER_ACK_FIELD; bitIndex++)
	{
		if (IsBitSet(header->previousReceivedAckBitfield, bitIndex))
		{
			ushort ackID = GetAckIDForBitfieldIndex(bitIndex, header->mostRecentReceivedAck);
			AckBundle& currBundle = GetAckForID(ackID);

			RemoveReceivedReliablesForBundle(currBundle);
		}
	}
}


//-----------------------------------------------------------------------------------------------
AckBundle& NetConnection::GetAckForID(ushort ackID)
{
	if (ackID >= m_currentZerothAckBundleIndex)
	{
		return m_ackBundles[ackID - m_currentZerothAckBundleIndex];
	}

	//We wrapped and this ack is from the previous cycle, so offset the result by the size of the array
	ushort newIndex = ackID + ARRAY_LENGTH(m_ackBundles) - m_currentZerothAckBundleIndex;

	return m_ackBundles[newIndex];
}


//-----------------------------------------------------------------------------------------------
void NetConnection::RemoveReceivedReliablesForBundle(const AckBundle& bundle)
{
	for (auto messageIter = m_unconfirmedReliables.begin(); messageIter != m_unconfirmedReliables.end();)
	{
		NetMessage& thisMessage = *messageIter;
		ushort reliableID = thisMessage.GetReliableID();
		bool foundID = false;
		for (ushort toConfirm : bundle.reliableIDs)
		{
			if (toConfirm == reliableID)
			{
				foundID = true;
				break;
			}
		}
		if (foundID)
		{
			messageIter = m_unconfirmedReliables.erase(messageIter);
		}
		else
		{
			messageIter++;
		}
	}
}


//-----------------------------------------------------------------------------------------------
bool NetConnection::UpdateExpectedReliablesAndCheckShouldProcess(ushort reliableID)
{
	if (reliableID < m_nextExpectedReliableID)
	{
		return false;
	}

	if (reliableID > m_nextExpectedReliableID)
	{
		if (m_receivedReliablesPastExpected.find(reliableID) != m_receivedReliablesPastExpected.end())
		{
			return false;
		}
		else
		{
			m_receivedReliablesPastExpected.insert(reliableID);
			return true;
		}
	}

	//This code assumes equality.  We must update the next expected id based on our previous received ids
	//and remove any previous received now less than the new next expected.
	//Optimization depends on the implementation of the set, which SHOULD be ordered least to greatest
	while (!m_receivedReliablesPastExpected.empty())
	{
		m_nextExpectedReliableID++;

		auto iter = m_receivedReliablesPastExpected.begin();
		ushort testReliableID = *iter;

		if (testReliableID != m_nextExpectedReliableID)
		{
			break;
		}

		m_receivedReliablesPastExpected.erase(iter);
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
void NetConnection::CheckShouldSendHeartbeat(float deltaSeconds)
{
	m_timeSinceLastHeartbeatSent += deltaSeconds;

	switch (m_type)
	{
		//These connection types will never send heartbeats, so we'll reset the counter and do nothing
	case NETCONNECTIONTYPE_UNCONFIRMED:
	case NETCONNECTIONTYPE_LOCAL:
		m_timeSinceLastHeartbeatSent = 0.f;
		return;
	}

	//I was only going to do this for BAD connections, but I ended up with one-sided stuff, so I send every 5 seconds regardless
	if (m_timeSinceLastHeartbeatSent > TIME_UNTIL_HEARTBEAT)
	{
		NetMessage heartbeat(NETMESSAGE_HEARTBEAT);
		AddMessage(heartbeat);
		m_timeSinceLastHeartbeatSent = 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
QuString NetConnection::GetDebugString() const
{
	QuString result = "";

	result += "Connection: ";
	if (IsHost())
	{
		result += "Host";
	}
	result += "\n";
	result += QuString::F("    Last received message timestamp: %u\n", m_lastReceivedTimestamp);
	result += QuString::F("    Index: %i\n", m_index);
	result += QuString::F("    Address: %s\n", Network::GetStringFromAddr((sockaddr*)&m_toAddr));
	result += QuString::F("    GUID : %s\n", m_guid.c_str());
	result += QuString::F("    Time since last sent packet: %.2fs\n", m_timeSinceLastSentPacket);
	result += QuString::F("    Time since last received packet: %.2fs\n", m_timeSinceLastReceivedPacket);
	result += QuString::F("    Current outgoing packet ack: %u\n", (m_currentAckIndex) ? m_currentAckIndex : INVALID_PACKET_ACK);
	result += QuString::F("    Most recently confirmed ack: %u\n", m_highestReceivedAck);
	result += "    Previous confirmed ack bitfield: ";
	for (int i = BITS_PER_ACK_FIELD - 1; i >= 0; i--)
	{
		if (((1 << i) & m_previousReceivedAckBitfield) == 0)
		{
			result += "0";
		}
		else
		{
			result += "1";
		}
	}
	result += "\n";

	return result;
}


//-----------------------------------------------------------------------------------------------
void NetConnection::InitializeVoiceSystem()
{
//	m_voiceChatSystem.Initialize();

//	g_eventSystem->RegisterEvent<NetConnection, &NetConnection::OnSoundBiteComplete>("OnSoundBiteComplete", this);
}


//-----------------------------------------------------------------------------------------------
//void NetConnection::OnSoundBiteComplete(Event* e)
//{
//	SoundBiteData* data = (SoundBiteData*)e;
//	const ushort numBytesPerMessage = LARGE_MESSAGE_CHUNK_BYTES;
//
//	size_t numChunks = BYTES_PER_BITE / numBytesPerMessage;
//	ushort remainingBytes = BYTES_PER_BITE % numBytesPerMessage;
//
//	if (remainingBytes)
//	{
//		numChunks++;
//	}
//
//	for (size_t i = 0; i < numChunks; i++)
//	{
//		NetMessage voiceChunk(NETMESSAGE_VOICE_CHUNK);
//		voiceChunk.Write<float>(data->soundbite->timestamp);
//		voiceChunk.Write<ushort>(data->biteIndex);
//		voiceChunk.Write<size_t>(i);
//		voiceChunk.Write<size_t>(numChunks);
//		ushort numBytes = numBytesPerMessage;
//		if (i == numChunks - 1 && remainingBytes)
//		{
//			numBytes = remainingBytes;
//		}
//		voiceChunk.WriteBuffer(data->soundbite->buffer + i * numBytesPerMessage, numBytes);
//
//		AddMessage(voiceChunk);
//	}
//}
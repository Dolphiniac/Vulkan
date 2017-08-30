#include "Engine/Network/NetPacket.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
bool NetPacket::WriteContents(const NetMessage& msg)
{
	MessageHeader msgHeader;
	msgHeader.flags = msg.GetFlags();
	msgHeader.type = msg.GetMessageType();
	uint16_t payloadSize = msg.GetSize();
	msgHeader.messageSize = sizeof(MessageHeader) + payloadSize;
	msgHeader.reliableID = msg.GetReliableID();
	msgHeader.sequenceID = msg.GetSequenceID();
	if (GetWritableBytes() < msgHeader.messageSize)
	{
		return false;
	}

	Write<MessageHeader>(msgHeader);
	//Messages should already be in correct byte order, so write them forward
	BytePacker::WriteForward(msg.GetContents(), (void**)&m_currPtr, payloadSize);

	//Increase message count by one
	PacketHeader* header = (PacketHeader*)m_buffer;
	header->messageCount++;

	return true;
}


//-----------------------------------------------------------------------------------------------
void NetPacket::ReadContents(byte* outBuffer, uint16_t messageSize)
{
	BytePacker::ReadForward(outBuffer, (void**)&m_currPtr, messageSize);

	m_remainingMessages--;
}


//-----------------------------------------------------------------------------------------------
uint16_t NetPacket::Advance(int numBytes)
{
	numBytes = Clampi(numBytes, 0, GetWritableBytes());
	
	m_currPtr += numBytes;

	return (uint16_t)numBytes;
}


//-----------------------------------------------------------------------------------------------
void NetPacket::Initialize(int bufferSize)
{
	PacketHeader* header = ReadPacketHeaderAndAdvance();
	byte numMessages = header->messageCount;
	int packetSize = sizeof(PacketHeader);
	for (int i = 0; i < numMessages; i++)
	{
		MessageHeader* msgHeader = ReadMessageHeaderAndAdvance();
		packetSize += msgHeader->messageSize;
		Advance(msgHeader->messageSize - sizeof(MessageHeader));
	}

	ASSERT_OR_DIE(bufferSize == packetSize, "Packet corrupted. Buffer size does not match packet read size");

	//Rewind read
	m_currPtr = m_buffer + sizeof(PacketHeader);
	m_remainingMessages = numMessages;
}
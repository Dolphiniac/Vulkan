#include "Engine/Network/PacketChannel.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
PacketChannel::PacketChannel(SOCKET sock)
	: m_sock(sock)
	, m_loss(0.f)
	, m_lag(0, 0)
{
	g_eventSystem->RegisterEvent<PacketChannel, &PacketChannel::Tick>("Tick", this);
}


//-----------------------------------------------------------------------------------------------
void PacketChannel::Tick(Event* e)
{
	TickEvent* te = (TickEvent*)e;
	float deltaSeconds = te->deltaSeconds;

	//For each lagged packet, tick it down on the multimap by deltaSeconds
	std::multimap<float, PacketInfo> oldLaggedPackets;
	std::swap(oldLaggedPackets, m_laggedPackets);
	for (std::pair<const float, PacketInfo>& p : oldLaggedPackets)
	{
		m_laggedPackets.insert(std::make_pair(p.first - deltaSeconds, p.second));
	}
}


//-----------------------------------------------------------------------------------------------
int PacketChannel::SendTo(const char* buffer, size_t bytes, int flags, const sockaddr_in* toAddress)
{
	return GLOBAL::sendto(m_sock, buffer, bytes, flags, (sockaddr*)toAddress, sizeof(sockaddr_in));
}


//-----------------------------------------------------------------------------------------------
int PacketChannel::RecvFrom(char* buffer, size_t maxBytes, int flags, sockaddr_in* outAddr, int* outAddrLen)
{
	PacketInfo packReceived;
	memcpy(&packReceived.addrlen, outAddrLen, sizeof(int));
	packReceived.bytes = GLOBAL::recvfrom(m_sock, packReceived.buffer, maxBytes, flags, (sockaddr*)&packReceived.addr, &packReceived.addrlen);

	//Passthrough for no packet
	if (packReceived.bytes > 0)
	{
		float lossFloat = GetRandomNormalized();
		//If random value is inside loss percentage, drop the packet
		if (lossFloat >= m_loss)
		{
			//Get random lag and insert packet into multimap (sorted by key, and then by order of insertion)
			int lagMilliseconds = m_lag.GetRandom();
			float lagSeconds = (float)lagMilliseconds * .001f;

			m_laggedPackets.insert(std::make_pair(lagSeconds, packReceived));
		}
	}


	//If first packet has exhausted its timeout, copy it out, as if it were the one received
	auto iter = m_laggedPackets.begin();
	if (iter != m_laggedPackets.end())
	{
		if (iter->first <= 0.f)
		{
			memcpy(buffer, iter->second.buffer, iter->second.bytes);
			memcpy(outAddr, &iter->second.addr, iter->second.addrlen);
			memcpy(outAddrLen, &iter->second.addrlen, sizeof(int));

			int result = iter->second.bytes;

			m_laggedPackets.erase(iter);

			return result;
		}
	}

	return 0;
}


//-----------------------------------------------------------------------------------------------
QuString PacketChannel::GetDebugString() const
{
	QuString result = "";

	int min;
	int max;
	m_lag.GetRangeValues(min, max);
	result += QuString::F("Simulated lag: %ims to %ims\n", min, max);
	result += QuString::F("Simulated loss: %i%%\n", (int)(m_loss * 100.f));

	return result;
}
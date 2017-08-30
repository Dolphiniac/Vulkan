#pragma once

#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Math/Range.hpp"
#include "Engine/Network/NetMessage.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Quantum/Core/String.h"

#include <map>

struct PacketInfo
{
	char buffer[UDP_PACKET_MAX_LENGTH];
	sockaddr_in addr;
	int addrlen;
	int bytes;
};


//-----------------------------------------------------------------------------------------------
class PacketChannel
{
public:
	PacketChannel(SOCKET sock);
	~PacketChannel() { closesocket(m_sock); }
	int SendTo(const char* buffer, size_t bytes, int flags, const sockaddr_in* toAddress);
	int RecvFrom(char* buffer, size_t maxBytes, int flags, sockaddr_in* outAddr, int* outAddrLen);
	void SetLag(int minMilliSeconds, int maxMilliSeconds) { m_lag.SetRange(minMilliSeconds, maxMilliSeconds); }
	void SetLoss(float loss) { m_loss = loss; }
	void Tick(Event*);
	QuString GetDebugString() const;

private:
	SOCKET m_sock;
	float m_loss;
	Range<int> m_lag;
	std::multimap<float, PacketInfo> m_laggedPackets;
};
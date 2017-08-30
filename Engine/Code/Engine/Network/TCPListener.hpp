#pragma once

#include "Engine/Network/NetworkSystem.hpp"


//-----------------------------------------------------------------------------------------------
class TCPListener
{
public:
	TCPListener(SOCKET sock, const sockaddr_in& addr);
	class TCPConnection* AcceptConnection();
	bool IsValid() const { return m_isValid; }
	const char* GetConnectionString();
	~TCPListener();

private:
	SOCKET m_sock;
	sockaddr_in m_addr;
	bool m_isValid;
};
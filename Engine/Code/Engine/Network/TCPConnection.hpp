#pragma once

#include "Engine/Network/NetworkSystem.hpp"
#include <string>


//-----------------------------------------------------------------------------------------------
class TCPConnection
{
public:
	TCPConnection(SOCKET sock, const sockaddr_in& addr, const char* name = nullptr);
	std::string GetConnectionInfo();
	void ReceiveMessages();
	bool IsValid() const { return m_isValid; }
	void SendCommand(const std::string& command);

private:
	std::string m_currCommand;
	const char* m_name;
	SOCKET m_sock;
	sockaddr_in m_addr;
	bool m_isValid;
};
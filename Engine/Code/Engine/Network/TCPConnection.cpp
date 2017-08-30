#include "Engine/Network/TCPConnection.hpp"
#include "Engine/Core/ConsoleCommand.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
TCPConnection::TCPConnection(SOCKET sock, const sockaddr_in& addr, const char* name /* = nullptr */)
	: m_sock(sock)
	, m_name(name)
	, m_isValid(true)
{
	memcpy(&m_addr, &addr, sizeof(addr));
}


//-----------------------------------------------------------------------------------------------
std::string TCPConnection::GetConnectionInfo()
{
	std::string result;

	if (m_name)
	{
		result = m_name;
	}
	else
	{
		m_name = "";
	}

	result += Network::GetStringFromAddr((sockaddr*)&m_addr);

	return result;
}


//-----------------------------------------------------------------------------------------------
void TCPConnection::ReceiveMessages()
{
	char buffer[1 KB];

	int numReceived = recv(m_sock, buffer, 1 KB, 0);

	for (int i = 0; i < numReceived; i++)
	{
		char c = buffer[i];
		m_currCommand.push_back(c);
		if (c == NULL)
		{
			std::string substring = m_currCommand.substr(1);
			ConsolePrintf(WHITE, "Remote: %s", substring.c_str());
			ConsoleCommand cc(substring);
			cc.CallFunc();
			m_currCommand.clear();
			m_currCommand.shrink_to_fit();
		}
	}
// 	WSAPOLLFD fd;
// 	fd.fd = m_sock;
// 	int result = WSAPoll(&fd, 1, 10);
// 	if (result == SOCKET_ERROR)
// 	{
// 		if (Network::ShouldDisconnect(WSAGetLastError()))
// 		{
// 			m_isValid = false;
// 		}
// 	}
}


//-----------------------------------------------------------------------------------------------
void TCPConnection::SendCommand(const std::string& command)
{
	char msgType = '0';
	send(m_sock, &msgType, 1, 0);
	send(m_sock, command.c_str(), command.size(), 0);
	char endOfMsg = NULL;
	send(m_sock, &endOfMsg, 1, 0);
}
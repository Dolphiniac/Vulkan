#include "Engine/Network/TCPListener.hpp"
#include "Engine/Network/TCPConnection.hpp"


//-----------------------------------------------------------------------------------------------
TCPListener::TCPListener(SOCKET sock, const sockaddr_in& addr)
	: m_sock(sock)
	, m_isValid(true)
{
	memcpy(&m_addr, &addr, sizeof(addr));
}


//-----------------------------------------------------------------------------------------------
TCPConnection* TCPListener::AcceptConnection()
{
	sockaddr_storage addr;
	int addrlen = sizeof(addr);
	SOCKET acceptedSocket = accept(m_sock, (sockaddr*)&addr, &addrlen);

	if (acceptedSocket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		if (Network::ShouldDisconnect(error))
		{
			m_isValid = false;
		}
		return nullptr;
	}
	sockaddr_in addrIn;
	memcpy(&addrIn, &addr, addrlen);
	TCPConnection* result = new TCPConnection(acceptedSocket, addrIn);

	return result;
}


//-----------------------------------------------------------------------------------------------
const char* TCPListener::GetConnectionString()
{
	return Network::GetStringFromAddr((sockaddr*)&m_addr);
}


//-----------------------------------------------------------------------------------------------
TCPListener::~TCPListener()
{
	closesocket(m_sock);
}
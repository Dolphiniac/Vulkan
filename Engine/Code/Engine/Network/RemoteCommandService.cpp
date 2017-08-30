#include "Engine/Network/RemoteCommandService.hpp"
#include "Engine/Core/ConsoleCommand.hpp"
#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Network/TCPConnection.hpp"
#include "Engine/Network/TCPListener.hpp"

#define RCS_PORT "4325"

RemoteCommandService* g_remoteCommandService = nullptr;


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(ListAddresses, args)
{
	std::string port = args.GetNextArg();
	if (port == "")
	{
		ConsolePrint("Port number missing", RED);
		return;
	}
	int portNum = 0; 
	try
	{
		portNum = std::stoi(port);
	}
	catch (const std::invalid_argument& ia)
	{
		UNUSED(ia);
		ConsolePrint("Port number invalid", RED);
		return;
	}
	const char* host = Network::GetLocalHostName();
	addrinfo* addresses = Network::AllocAddressesForHost(host, port.c_str(), AF_INET, SOCK_STREAM);
	addrinfo* iter = addresses;
	while (iter)
	{
		ConsolePrintf(WHITE, "%s:%s", Network::GetStringFromAddr(iter->ai_addr), port.c_str());
		iter = iter->ai_next;
	}
	Network::FreeAddresses(addresses);
}


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(CSHost, args)
{
	if (g_remoteCommandService)
	{
		ConsolePrint("Cannot start remote command service session!  Already exists", RED);
	}
	g_remoteCommandService = new RemoteCommandService();
	std::string host = args.GetNextArg();
	if (host == "")
	{
		ConsolePrint("Host missing", RED);
		return;
	}
	addrinfo* addresses = Network::AllocAddressesForHost(host.c_str(), RCS_PORT, AF_INET, SOCK_STREAM, AI_PASSIVE);
	addrinfo* iter = addresses;
	while (iter)
	{
		SOCKET sock = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
		if (sock != INVALID_SOCKET)
		{
			int result = bind(sock, iter->ai_addr, iter->ai_addrlen);
			if (result == SOCKET_ERROR)
			{
				closesocket(sock);
			}
			else
			{
				u_long nonBlocking = 1;
				ioctlsocket(sock, FIONBIO, &nonBlocking);
				listen(sock, 2);
				sockaddr_in addr;
				memcpy(&addr, iter->ai_addr, iter->ai_addrlen);
				g_remoteCommandService->SetListener(new TCPListener(sock, addr));
				ConsolePrint("Host successful!", WHITE);
				goto cleanup;
			}
		}
		iter = iter->ai_next;
	}
	ConsolePrint("Could not host", RED);

cleanup:
	Network::FreeAddresses(addresses);
}


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(CSStop, args)
{
	UNUSED(args);

	SAFE_DELETE(g_remoteCommandService);
}


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(CSJoin, args)
{
	if (g_remoteCommandService)
	{
		ConsolePrint("Cannot start remote command service session!  Already exists", RED);
		return;
	}
	g_remoteCommandService = new RemoteCommandService();
	std::string host = args.GetNextArg();
	if (host == "")
	{
		ConsolePrint("Host missing", RED);
		return;
	}

	addrinfo* addresses = Network::AllocAddressesForHost(host.c_str(), RCS_PORT, AF_INET, SOCK_STREAM);
	addrinfo* iter = addresses;
	while (iter)
	{
		SOCKET sock = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
		if (sock != INVALID_SOCKET)
		{
			int result = connect(sock, iter->ai_addr, iter->ai_addrlen);

			if (result == SOCKET_ERROR)
			{
				closesocket(sock);
			}
			else
			{
				u_long nonBlocking = 1;
				ioctlsocket(sock, FIONBIO, &nonBlocking);
				sockaddr_in addr;
				memcpy(&addr, iter->ai_addr, iter->ai_addrlen);

				g_remoteCommandService->AddConnection(new TCPConnection(sock, addr));
				goto cleanup;
			}
		}

		iter = iter->ai_next;
	}

	ConsolePrint("Could not connect", RED);

cleanup:
	Network::FreeAddresses(addresses);
}


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(CSLeave, args)
{
	UNUSED(args);

	SAFE_DELETE(g_remoteCommandService);
}


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(CSInfo, args)
{
	UNUSED(args);

	std::vector<std::string> info = g_remoteCommandService->GetCommandServiceInfo();
	for (const std::string& infoLine : info)
	{
		ConsolePrint(infoLine, WHITE);
	}
}


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(CSSend, args)
{
	std::string fullCommand = "";
	std::string arg = args.GetNextArg();
	while (arg != "")
	{
		fullCommand += arg + " ";
		arg = args.GetNextArg();
	}

	g_remoteCommandService->SendCommand(fullCommand);
}


//-----------------------------------------------------------------------------------------------
void RemoteCommandService::Update()
{
	if (m_listener)
	{
		TCPConnection* conn = m_listener->AcceptConnection();
		if (conn)
		{
			AddConnection(conn);
			ConsolePrintf(WHITE, "Connection to %s accepted", conn->GetConnectionInfo().c_str());
		}
		if (!m_listener->IsValid())
		{
			this->~RemoteCommandService();
		}
	}

	for (auto iter = m_activeConnections.begin(); iter != m_activeConnections.end();)
	{
		TCPConnection* conn = *iter;
		if (conn->IsValid())
		{
			conn->ReceiveMessages();
			iter++;
		}
		else
		{
			delete conn;
			iter = m_activeConnections.erase(iter);
		}
	}
}


//-----------------------------------------------------------------------------------------------
std::vector<std::string> RemoteCommandService::GetCommandServiceInfo()
{
	std::vector<std::string> result;

	if (m_listener)
	{
		result.push_back("SERVER");
		result.push_back(m_listener->GetConnectionString());
		for (TCPConnection* conn : m_activeConnections)
		{
			result.push_back(conn->GetConnectionInfo());
		}
	}
	else if (!m_activeConnections.empty())
	{
		result.push_back("CLIENT");
		result.push_back(m_activeConnections[0]->GetConnectionInfo());
	}
	else
	{
		result.push_back("NONE");
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
void RemoteCommandService::SendCommand(const std::string& command)
{
	for (TCPConnection* conn : m_activeConnections)
	{
		conn->SendCommand(command);
	}
}


//-----------------------------------------------------------------------------------------------
RemoteCommandService::~RemoteCommandService()
{
	SAFE_DELETE(m_listener);
	for (TCPConnection* conn : m_activeConnections)
	{
		delete conn;
	}
	m_activeConnections.clear();
	m_activeConnections.shrink_to_fit();
}
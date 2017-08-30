#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

#pragma comment (lib, "ws2_32")


//-----------------------------------------------------------------------------------------------
void Network::SystemStartup()
{
	WSADATA data;
	int error = WSAStartup(MAKEWORD(2, 2), &data);
	ASSERT_OR_DIE(!error, "WSAStortup failed");
}


//-----------------------------------------------------------------------------------------------
void Network::SystemShutdown()
{
	int error = WSACleanup();
	ASSERT_OR_DIE(!error, "WSACleanup failed");
}


//-----------------------------------------------------------------------------------------------
const char* Network::GetLocalHostName()
{
	static char buffer[256];
	int error = gethostname(buffer, 256);
	if (!error)
	{
		return buffer;
	}
	else
	{
		return "localhost";
	}
}


//-----------------------------------------------------------------------------------------------
void Network::GetPortString(int portNum, char* buffer)
{
	//Port nums can't be longer than 5 chars, so using a magic number here XD
	GLOBAL::_itoa_s(portNum, buffer, 6, 10);
}


//-----------------------------------------------------------------------------------------------
addrinfo* Network::AllocAddressesForHost(const char* host, const char* serviceOrPort, int family, int sockType, int flags /* = 0 */)
{
	addrinfo hintAddr;
	memset(&hintAddr, 0, sizeof(addrinfo));

	hintAddr.ai_socktype = sockType;
	hintAddr.ai_family = family;
	hintAddr.ai_flags = flags;

	addrinfo* result = nullptr;
	int error = getaddrinfo(host, serviceOrPort, &hintAddr, &result);

	if (error)
	{
		return nullptr;
	}
	
	return result;
}


//-----------------------------------------------------------------------------------------------
SOCKET Network::CreateUDPSocket(const char* addr, const char* port, sockaddr_in* outAddr)
{
	addrinfo* addresses = AllocAddressesForHost(addr, port, AF_INET, SOCK_DGRAM, AI_PASSIVE);
	SOCKET result = INVALID_SOCKET;
	addrinfo* iter = addresses;

	while (iter)
	{
		result = GLOBAL::socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
		if (result != INVALID_SOCKET)
		{
			int bindResult = GLOBAL::bind(result, iter->ai_addr, iter->ai_addrlen);
			if (bindResult != SOCKET_ERROR)
			{
				SetNonBlocking(result);
				ASSERT_OR_DIE(iter->ai_addrlen == sizeof(sockaddr_in), "Bad addrlen on UDP socket creation");
				if (outAddr)
				{
					memcpy(outAddr, iter->ai_addr, iter->ai_addrlen);
				}
				break;
			}
			else
			{
				GLOBAL::closesocket(result);
				result = INVALID_SOCKET;
			}
		}
		iter = iter->ai_next;
	}

	Network::FreeAddresses(addresses);

	return result;
}


//-----------------------------------------------------------------------------------------------
void Network::SetNonBlocking(SOCKET sock)
{
	u_long nonBlocking = 1;
	ioctlsocket(sock, FIONBIO, &nonBlocking);
}


//-----------------------------------------------------------------------------------------------
void Network::FreeAddresses(addrinfo* addresses)
{
	if (addresses)
	{
		freeaddrinfo(addresses);
	}
}


//-----------------------------------------------------------------------------------------------
const char* Network::GetStringFromAddr(const sockaddr* addr)
{
	static char buffer[256];

	sockaddr_in* inAddr = (sockaddr_in*)addr;
	inet_ntop(inAddr->sin_family, (void*)&inAddr->sin_addr, buffer, 256);

	return buffer;
}


//-----------------------------------------------------------------------------------------------
const char* Network::GetFullStringFromAddr(const sockaddr* addr)
{
	static char buffer[256];

	sockaddr_in* inAddr = (sockaddr_in*)addr;
	const char* inetAddr = GetStringFromAddr(addr);
	int addrlen = strlen(inetAddr);
	strcpy_s(buffer, addrlen + 1, inetAddr);
	buffer[addrlen] = ':';
	_itoa_s(ntohs(inAddr->sin_port), &buffer[addrlen + 1], 6, 10);

	return buffer;
}


//-----------------------------------------------------------------------------------------------
bool Network::GetAddrFromString(const char* string, sockaddr_in* outAddr)
{
	outAddr->sin_family = AF_INET;
	std::string workingString = string;
	std::vector<std::string> tokens = SplitOnDelimiter(workingString, ':');
	if (tokens.size() != 2)
	{
		return false;
	}
	addrinfo* addresses = AllocAddressesForHost(tokens[0].c_str(), tokens[1].c_str(), AF_INET, SOCK_DGRAM);
	bool success = (addresses != nullptr);
	memcpy(outAddr, addresses->ai_addr, addresses->ai_addrlen);
	FreeAddresses(addresses);
	return success;
// 	int result = inet_pton(AF_INET, tokens[0].c_str(), (void*)&outAddr->sin_addr);
// 	if (result != 1 /* Valid return val */)
// 	{
// 		return false;
// 	}
// 
// 	try
// 	{
// 		outAddr->sin_port = htons(std::stol(tokens[1]));
// 	}
// 	catch (const std::invalid_argument&)
// 	{
// 		return false;
// 	}
// 
// 	return true;
}


//-----------------------------------------------------------------------------------------------
bool Network::ShouldDisconnect(int errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAEMSGSIZE:
	case WSAEWOULDBLOCK:
		return false;

	default:
		return true;
	}
}


//-----------------------------------------------------------------------------------------------
bool Network::AreSameAddress(const sockaddr_in& first, const sockaddr_in& second)
{
	return first.sin_addr.S_un.S_addr == second.sin_addr.S_un.S_addr && first.sin_port == second.sin_port;
}
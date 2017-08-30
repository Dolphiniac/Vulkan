#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>

#define LITTLE_ENDIAN 0
#define BIG_ENDIAN 1

//Global expected endianness for network traffic
#define NET_TRAFFIC_ENDIANNESS LITTLE_ENDIAN
#define INVALID_CONNECTION_INDEX 0xFF


//-----------------------------------------------------------------------------------------------
typedef byte Endianness;


//-----------------------------------------------------------------------------------------------
namespace Network
{

	void SystemStartup();
	void SystemShutdown();
	const char* GetLocalHostName();
	void GetPortString(int portNum, char* buffer);
	addrinfo* AllocAddressesForHost(const char* host, const char* serviceOrPort, int family, int sockType, int flags = 0);
	SOCKET CreateUDPSocket(const char* addr, const char* port, sockaddr_in* outAddr);
	void SetNonBlocking(SOCKET sock);
	void FreeAddresses(addrinfo* addresses);
	const char* GetStringFromAddr(const sockaddr* addr);

	//Different version for addr with port
	const char* GetFullStringFromAddr(const sockaddr* addr);
	bool GetAddrFromString(const char* string, sockaddr_in* outAddr);
	bool ShouldDisconnect(int errorCode);
	inline Endianness GetEngineEndianness();
	bool AreSameAddress(const sockaddr_in& first, const sockaddr_in& second);
}


//-----------------------------------------------------------------------------------------------
inline Endianness Network::GetEngineEndianness()
{
	union converter
	{
		int conI;
		char conB[4];
	};
	converter c;
	c.conI = 1;
	return (c.conB[0] == 0) ? BIG_ENDIAN : LITTLE_ENDIAN;
}
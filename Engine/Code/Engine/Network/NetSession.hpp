#pragma once
#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Network/NetMessage.hpp"
#include "Engine/Network/NetConnection.hpp"
#include "Engine/Network/NetPacket.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Network/PacketChannel.hpp"
#include "Quantum/Core/String.h"

#include <string>
#include <map>
#include <vector>


//-----------------------------------------------------------------------------------------------
enum ENetSessionState
{
	NETSESSIONSTATE_INVALID = 0,
	NETSESSIONSTATE_SETUP,		//Not used, instantaneous
	NETSESSIONSTATE_DISCONNECTED,
	NETSESSIONSTATE_HOSTING,	//You shouldn't ever see this unless Host fails on no self-connection
	NETSESSIONSTATE_JOINING,
	NETSESSIONSTATE_CONNECTED
};


//-----------------------------------------------------------------------------------------------
enum ENetErrorType : byte
{
	NETERROR_NONE = 0,
	NETERROR_SOCKET_CREATE_FAILED,
	NETERROR_JOIN_HOST_TIMEOUT,
	NETERROR_JOIN_DENIED_NO_NEW_CONNECTIONS,
	NETERROR_JOIN_DENIED_NOT_HOST,
	NETERROR_JOIN_DENIED_FULL,
	NETERROR_JOIN_DENIED_GUID_IN_USE,
	NETERROR_HOST_DISCONNECTED
};


//-----------------------------------------------------------------------------------------------
extern class NetSession* g_netSession;


//-----------------------------------------------------------------------------------------------
struct NetSender
{
	NetSession* session;
	sockaddr address;
	NetConnection* connection;
	ushort ackID;	//For reliable connectionless, we can send an ack right back
};


//-----------------------------------------------------------------------------------------------
struct NetworkTickEvent : Event
{
	NetConnection* connection;
};


//-----------------------------------------------------------------------------------------------
struct ConnectionChangeEvent : Event
{
	NetConnection* connection;
};



//-----------------------------------------------------------------------------------------------
class NetSession
{
	friend class NetConnection;

	static const float NETWORK_TICK_INTERVAL;
	static const float TIME_UNTIL_JOIN_TIMEOUT;
public:
	NetSession();
	~NetSession();
	bool Start(const int portNum, const int maxDistanceFromPort = 8);
	void Stop();
	std::string GetAddressString();
	void SendMessageDirect(const sockaddr_in* dest, NetMessage& msg);
	void SendPacketDirect(const sockaddr_in* dest, const NetPacket& packet);
	void Update();
	void Tick(Event* e);
	void RegisterMessage(ENetMessage type, const char* debugName, OnMessageReceiveFunc callback);
	void RegisterCoreMessages();
	bool IsMe(const sockaddr_in& otherAddr) const;
	bool IsMe(const NetConnection* connection) const { return connection == m_myConnection; }
	void AddConnection(NetConnection* nc);
	void RemoveConnection(NetConnection* nc);
	bool DoesConnectionExist(const NetConnection* nc);
	bool IsIndexTaken(byte index) const;
	bool IsGUIDTaken(const std::string& guid);
	void DestroyConnection(byte index);
	void SetOwnConnectionInfo(int index, const std::string& guid);
	NetConnection* GetOwnConnection() const { return m_myConnection; }
	NetConnection* GetConnectionAtIndex(byte index) const;
	NetConnection* FindConnectionWithAddr(const sockaddr_in& address) const;
	void SetLoss(float lossPercentage) { m_packetChannel->SetLoss(lossPercentage); }
	void SetLag(int minMilliseconds, int maxMilliseconds) { m_packetChannel->SetLag(minMilliseconds, maxMilliseconds); }
	std::vector<NetConnection*>& GetConnections() { return m_activeConnections; }
	QuString GetDebugString() const;
	ENetErrorType GetLastError() const { return m_lastError; }
	ENetSessionState GetSessionState() const { return m_state; }
	bool IsHost() const { return m_isHost; }
	void SendConnectionlessReliableResponseWithMessage(NetMessage& toSend, ushort ackID, const sockaddr& returnAddress);
	byte GetValidPlayerIndex() const;
	void SetError(ENetErrorType error) { m_lastError = error; }
	void OnJoinFail();
	void OnJoin();
	void FlushConnections();

	void Host(const char* username);
	void Join(const char* username, const sockaddr_in& addr);
	void Leave();

private:
	void ProcessPackets();
	bool ReadNextPacket(class NetPacket* packet, sockaddr* addr);
	bool ReadNextMessage(NetMessage* msg, NetPacket& packet, NetConnection* connection);
	NetMessageDef* GetDefinition(ENetMessage type) { if (type + 1U > m_definitions.size()) return nullptr; return m_definitions.at(type); }

private:
	NetConnection* m_myConnection;
	std::vector<NetConnection*> m_activeConnections;
	std::vector<NetMessageDef*> m_definitions;
	class PacketChannel* m_packetChannel;
	float m_timeSinceLastNetworkTick;
	float m_timeSinceLastPacketSent;
	float m_timeSinceLastPacketReceived;
	float m_joinAttemptTime;
	ENetSessionState m_state;
	ENetErrorType m_lastError;
	bool m_isHost;
};
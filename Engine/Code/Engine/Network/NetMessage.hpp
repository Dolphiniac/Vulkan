#pragma once

#include "Engine/Network/NetworkSystem.hpp"

#include <string>

#define UDP_PACKET_MAX_LENGTH 1232
#define LARGE_MESSAGE_CHUNK_BYTES 1 KB


//-----------------------------------------------------------------------------------------------
enum ENetMessageCore : byte
{
	NETMESSAGE_PING = 0,
	NETMESSAGE_PONG,
	NETMESSAGE_HEARTBEAT,
	NETMESSAGE_JOIN_REQUEST,
	NETMESSAGE_JOIN_DENY,
	NETMESSAGE_JOIN_ACCEPT,
	NETMESSAGE_LEAVE,
	NETMESSAGE_VOICE_SYNC,
	NETMESSAGE_VOICE_CHUNK,
	NETMESSAGE_CORE_COUNT
};
typedef byte ENetMessage;


//-----------------------------------------------------------------------------------------------
enum ENetMessageFlag : byte
{
	NETMESSAGEFLAG_CONNECTIONLESS = 0,
	NETMESSAGEFLAG_RELIABLE,
	NETMESSAGEFLAG_ORDERED
};


//-----------------------------------------------------------------------------------------------
struct MessageHeader
{
	uint16_t messageSize;
	ushort reliableID;
	ushort sequenceID;
	ENetMessage type;
	byte flags;
};

//-----------------------------------------------------------------------------------------------
typedef void(*OnMessageReceiveFunc)(struct NetSender& sender, class NetMessage& msg);


//-----------------------------------------------------------------------------------------------
struct NetMessageDef
{
	OnMessageReceiveFunc callback;
	const char* debugName;
};


//-----------------------------------------------------------------------------------------------
class NetMessage
{
	friend class NetSession;
	friend class NetConnection;
public:
	NetMessage(byte type);
	NetMessage(const NetMessage& other);
	void operator=(const NetMessage& other);
	template<typename T> void Write(const ND<T>& toWrite);
	template<typename T> bool Read(ND<T>& outData);
	void WriteBuffer(void* buffer, ushort numBytes);
	bool ReadBuffer(void* outBuffer, ushort& numBytes);
	uint16_t GetSize() const { return (uint16_t)(m_currPtr - m_buffer); }
	uint16_t GetWritableBytes() const { return (uint16_t)(UDP_PACKET_MAX_LENGTH - GetLength()); }
	uint16_t GetReadableBytes() const { return GetWritableBytes(); }
	int GetLength() const { return m_currPtr - m_buffer; }
	const void* GetContents() const { return m_buffer; }
	ENetMessage GetMessageType() const { return m_type; }
	ENetMessageFlag GetFlags() const { return (ENetMessageFlag)m_flags; }
	void Reset() { m_currPtr = m_buffer; }
	bool CheckFlag(ENetMessageFlag flag) const { return ((1 << flag) & m_flags) != 0; }
	void SetFlag(ENetMessageFlag flag) { m_flags |= (1 << flag); }
	void ClearFlag(ENetMessageFlag flag) { m_flags &= ~(1 << flag); }
	bool IsReliable() const { return CheckFlag(NETMESSAGEFLAG_RELIABLE); }
	ushort GetReliableID() const { return m_reliableID; }
	void SetReliableID(ushort reliableID) { m_reliableID = reliableID; }
	bool IsOrdered() const { return CheckFlag(NETMESSAGEFLAG_ORDERED); }
	ushort GetSequenceID() const { return m_sequenceID; }
	void SetSequenceID(ushort sequenceID) { m_sequenceID = sequenceID; }

private:
	NetMessage();
	void WriteString(const char* toWrite);
	char* ReadString(char* buffer);
	
private:
	byte m_buffer[UDP_PACKET_MAX_LENGTH];
	byte* m_currPtr;
	byte m_type;
	byte m_flags;
	ushort m_reliableID;
	ushort m_sequenceID;
};

#include "Engine/Network/NetMessage.inl"
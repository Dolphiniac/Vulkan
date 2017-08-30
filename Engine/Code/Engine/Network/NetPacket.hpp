#pragma once

#include "Engine/Network/NetMessage.hpp"
#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Core/BytePacker.hpp"

#define INVALID_PACKET_ACK 0xFFFF
#define BITS_PER_ACK_FIELD 16

struct PacketHeader
{
	byte playerIndex;
	byte messageCount;
	ushort thisAck;
	ushort mostRecentReceivedAck;
	ushort previousReceivedAckBitfield;
};


//-----------------------------------------------------------------------------------------------
class NetPacket
{
	friend class NetSession;
public:
	inline NetPacket(bool forWriting = true);
	template<typename T> void Write(const ND<T>& data);
	template<typename T> bool Read(ND<T>& outData) const;
	bool WriteContents(const NetMessage& msg);
	void ReadContents(byte* outBuffer, uint16_t messageSize);
	uint16_t GetWritableBytes() const { return (uint16_t)(UDP_PACKET_MAX_LENGTH - GetLength()); }
	uint16_t GetReadableBytes() const { return GetWritableBytes(); }
	const char* GetCopyableBuffer() const { return (const char*)m_buffer; }
	int GetLength() const { return m_currPtr - m_buffer; }
	uint16_t Advance(int numBytes);
	void Reset() { m_currPtr = m_buffer; }
	inline PacketHeader* ReadPacketHeaderAndAdvance();
	inline PacketHeader* GetPacketHeader() const { return (PacketHeader*)m_buffer; }
	inline MessageHeader* ReadMessageHeaderAndAdvance();

private:
	char* GetBuffer() { return (char*)m_buffer; }
	void Initialize(int bufferSize);

private:
	byte m_buffer[UDP_PACKET_MAX_LENGTH];
	byte* m_currPtr;
	int m_remainingMessages;
};


#include "Engine/Network/NetPacket.inl"
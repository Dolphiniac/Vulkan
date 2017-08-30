#include "Engine/Network/NetMessage.hpp"
#include "Engine/Core/BytePacker.hpp"
#include "Engine/Math/Vector3.hpp"

#include <string.h>

const unsigned char INVALID_STRING_TOKEN = 0xFF;


//-----------------------------------------------------------------------------------------------
NetMessage::NetMessage()
	: m_currPtr(m_buffer)
	, m_flags(0)
{

}


//-----------------------------------------------------------------------------------------------
NetMessage::NetMessage(byte type)
	: m_type(type)
	, m_flags(0)
	, m_currPtr(m_buffer)
{
	m_currPtr = m_buffer;
}


//-----------------------------------------------------------------------------------------------
NetMessage::NetMessage(const NetMessage& other)
{
	int numBytes = other.m_currPtr - other.m_buffer;
	memcpy(m_buffer, other.m_buffer, numBytes);
	m_currPtr = m_buffer + numBytes;
	m_type = other.m_type;
	m_flags = other.m_flags;
	m_reliableID = other.m_reliableID;
	m_sequenceID = other.m_sequenceID;
}


//-----------------------------------------------------------------------------------------------
void NetMessage::operator=(const NetMessage& other)
{
	int numBytes = other.m_currPtr - other.m_buffer;
	memcpy(m_buffer, other.m_buffer, numBytes);
	m_currPtr = m_buffer + numBytes;
	m_type = other.m_type;
	m_flags = other.m_flags;
	m_reliableID = other.m_reliableID;
	m_sequenceID = other.m_sequenceID;
}


//-----------------------------------------------------------------------------------------------
void NetMessage::WriteBuffer(void* buffer, ushort numBytes)
{
	Write<ushort>(numBytes);
	if (GetWritableBytes() >= numBytes)
	{
		if (Network::GetEngineEndianness() == NET_TRAFFIC_ENDIANNESS)
		{
			BytePacker::WriteForward(buffer, (void**)&m_currPtr, numBytes);
		}
		else
		{
			BytePacker::WriteBackward(buffer, (void**)&m_currPtr, numBytes);
		}
	}
}


//-----------------------------------------------------------------------------------------------
bool NetMessage::ReadBuffer(void* outBuffer, ushort& outBytes)
{
	Read<ushort>(outBytes);
	if (Network::GetEngineEndianness() == NET_TRAFFIC_ENDIANNESS)
	{
		BytePacker::ReadForward(outBuffer, (void**)&m_currPtr, outBytes);
	}
	else
	{
		BytePacker::WriteBackward(outBuffer, (void**)&m_currPtr, outBytes);
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
void NetMessage::WriteString(const char* toWrite)
{
	if (GetWritableBytes() < 1)
	{
		return;
	}
	if (!toWrite)
	{
		*m_currPtr = INVALID_STRING_TOKEN;
		m_currPtr++;
		return;
	}
	int toWriteLen = strlen(toWrite);
	if (GetWritableBytes() >= toWriteLen + 1)
	{
		if (Network::GetEngineEndianness() == NET_TRAFFIC_ENDIANNESS)
		{
			BytePacker::WriteForward(toWrite, (void**)&m_currPtr, toWriteLen);
		}
		else
		{
			BytePacker::WriteBackward(toWrite, (void**)&m_currPtr, toWriteLen);
		}

		//No matter what order, we want null char at end of string so strlen works
		Write<char>('\0');
	}
}


//-----------------------------------------------------------------------------------------------
char* NetMessage::ReadString(char* buffer)
{
	char peek = *m_currPtr;
	if (peek == INVALID_STRING_TOKEN)
	{
		m_currPtr++;
		return nullptr;
	}
	else
	{
		int numChars = strlen((char*)m_currPtr);
		if (Network::GetEngineEndianness() == NET_TRAFFIC_ENDIANNESS)
		{
			BytePacker::ReadForward(buffer, (void**)&m_currPtr, numChars);
		}
		else
		{
			BytePacker::ReadBackward(buffer, (void**)&m_currPtr, numChars);
		}
		
		Read<char>(buffer[numChars]);
	}

	return buffer;
}
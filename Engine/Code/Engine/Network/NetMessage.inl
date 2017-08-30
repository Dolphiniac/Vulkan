//-----------------------------------------------------------------------------------------------
// INLINE FILE FOR NETMESSAGE.HPP
//-----------------------------------------------------------------------------------------------

#include "Engine/Core/BytePacker.hpp"


//-----------------------------------------------------------------------------------------------
template<typename T>
void inline NetMessage::Write(const ND<T>& toWrite)
{
	int dataSize = sizeof(T);

	if (GetWritableBytes() >= dataSize)
	{
		if (Network::GetEngineEndianness() == NET_TRAFFIC_ENDIANNESS)
		{
			BytePacker::WriteForward(&toWrite, (void**)&m_currPtr, dataSize);
		}
		else
		{
			BytePacker::WriteBackward(&toWrite, (void**)&m_currPtr, dataSize);
		}
	}
}


//-----------------------------------------------------------------------------------------------
template<typename T>
bool inline NetMessage::Read(ND<T>& outResult)
{
	int dataSize = sizeof(T);

	if (GetReadableBytes() >= dataSize)
	{
		if (Network::GetEngineEndianness() == NET_TRAFFIC_ENDIANNESS)
		{
			BytePacker::ReadForward(&outResult, (void**)&m_currPtr, dataSize);
		}
		else
		{
			BytePacker::ReadBackward(&outResult, (void**)&m_currPtr, dataSize);

		}

		return true;
	}
	else
	{
		return false;
	}
}


//-----------------------------------------------------------------------------------------------
template<> inline void NetMessage::Write<const char*>(const char* const& data)
{
	WriteString(data);
}


//-----------------------------------------------------------------------------------------------
template<> inline void NetMessage::Write<char*>(char* const & data)
{
	WriteString(data);
}


//-----------------------------------------------------------------------------------------------
template<> inline void NetMessage::Write<char* const>(char* const& data)
{
	WriteString(data);
}


//-----------------------------------------------------------------------------------------------
template<> inline void NetMessage::Write<std::string>(const std::string& data)
{
	WriteString(data.c_str());
}


//-----------------------------------------------------------------------------------------------
template<> inline bool NetMessage::Read<const char*>(const char*& data) = delete;

//-----------------------------------------------------------------------------------------------
template<> inline bool NetMessage::Read<char*>(char*& data)
{
	return (ReadString(data) != nullptr);
}


//-----------------------------------------------------------------------------------------------
template<> inline bool NetMessage::Read<char* const>(char* const& data)
{
	return (ReadString(data) != nullptr);
}


//-----------------------------------------------------------------------------------------------
template<> inline bool NetMessage::Read<std::string>(std::string& data)
{
	char* buffer = (char*)malloc(UDP_PACKET_MAX_LENGTH);
	buffer = ReadString(buffer);
	if (!buffer)
	{
		data = "";
		free(buffer);
		return false;
	}

	data = buffer;
	free(buffer);
	return true;
}
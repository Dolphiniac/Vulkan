//-----------------------------------------------------------------------------------------------
// INLINE FILE FOR NETPACKET.HPP
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
NetPacket::NetPacket(bool forWriting /* = true */)
{
	m_currPtr = m_buffer;
	PacketHeader* header = (PacketHeader*)m_buffer;
	header->playerIndex = INVALID_CONNECTION_INDEX;
	header->messageCount = 0;
	
	if (forWriting)
	{
		header++;
		m_currPtr = (byte*)header;
	}
}

//-----------------------------------------------------------------------------------------------
template<typename T>
void NetPacket::Write(const ND<T>& data)
{
	int dataSize = sizeof(T);

	if (GetWritableBytes() >= dataSize)
	{
		if (Network::GetEngineEndianness() == NET_TRAFFIC_ENDIANNESS)
		{
			BytePacker::WriteForward(&data, (void**)&m_currPtr, dataSize);
		}
		else
		{
			BytePacker::WriteBackward(&data, (void**)&m_currPtr, dataSize);
		}
	}
}


//-----------------------------------------------------------------------------------------------
template<typename T>
bool NetPacket::Read(ND<T>& outResult) const
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
PacketHeader* NetPacket::ReadPacketHeaderAndAdvance()
{
	PacketHeader* header = (PacketHeader*)m_currPtr;
	PacketHeader* advancePtr = header;
	advancePtr++;

	m_currPtr = (byte*)advancePtr;

	return header;
}


//-----------------------------------------------------------------------------------------------
MessageHeader* NetPacket::ReadMessageHeaderAndAdvance()
{
	MessageHeader* result = (MessageHeader*)m_currPtr;
	MessageHeader* advancePtr = result;
	advancePtr++;
	m_currPtr = (byte*)advancePtr;

	return result;
}
#include "Engine/Network/VoiceChatSystem.hpp"
#include "Engine/Core/EngineSystemManager.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Network/NetMessage.hpp"


////-----------------------------------------------------------------------------------------------
//VoiceChatSystem::VoiceChatSystem()
//	: m_timeConverter(0.f)
//{
//	memset(m_incomingBites, 0, sizeof(m_incomingBites));
//
//	for (int i = 0; i < ARRAY_LENGTH(m_incomingSoundBites); i++)
//	{
//		m_incomingSoundBites[i] = The.Audio->GetRecordingSound(BYTES_PER_BITE);
//	}
//	m_recordingSound = The.Audio->GetRecordingSound(BYTES_PER_BITE);
//}
//
//
////-----------------------------------------------------------------------------------------------
//VoiceChatSystem::~VoiceChatSystem()
//{
//	for (int i = 0; i < ARRAY_LENGTH(m_incomingSoundBites); i++)
//	{
//		m_incomingSoundBites[i]->release();
//	}
//	m_recordingSound->release();
//	g_eventSystem->UnregisterFromAllEvents(this);
//}
//
//
////-----------------------------------------------------------------------------------------------
//void VoiceChatSystem::Initialize()
//{
//	m_currentIncomingOffset = 0.f;
//	m_currentOutgoingBiteIndex = 0;
//	m_currentTime = 0.f;
//	m_currentBiteIndex = 0;
//	m_timeOnCurrentBite = 0.f;
//	m_timestampForCurrentIncoming = m_currentTime;
//	m_timestampForCurrentOutgoing = m_currentTime;
//	The.Audio->StartRecord(m_recordingSound);
//
//	g_eventSystem->RegisterEvent<VoiceChatSystem, &VoiceChatSystem::Tick>("Tick", this);
//}
//
//
////-----------------------------------------------------------------------------------------------
//void VoiceChatSystem::Tick(Event* e)
//{
//	TickEvent* te = (TickEvent*)e;
//	m_currentTime += te->deltaSeconds;
//	m_timeOnCurrentBite += te->deltaSeconds;
//
//	if (m_timeOnCurrentBite >= BITE_LENGTH)
//	{
//		SoundBite thisBites;
//
//		thisBites.timestamp = m_timestampForCurrentOutgoing;
//		m_timestampForCurrentOutgoing += BITE_LENGTH;
//		The.Audio->GetDataFromSound(thisBites.buffer, BYTES_PER_BITE, m_recordingSound);
//
//		SoundBiteData data;
//		data.soundbite = &thisBites;
//		data.biteIndex = m_currentOutgoingBiteIndex;
//		m_currentOutgoingBiteIndex++;
//		g_eventSystem->TriggerEvent("OnSoundBiteComplete", &data);
//		The.Audio->StartRecord(m_recordingSound);
//		m_timeOnCurrentBite -= BITE_LENGTH;
//	}
//
//	float readHead = m_currentTime - CLIENT_DELAY_FROM_INIT;
//	if (readHead >= m_timestampForCurrentIncoming)
//	{
//		int lastIndex = m_currentBiteIndex;
//		DecrementIndexWrapped(lastIndex, BUFFERED_BITE_COUNT);
//		memset(&m_incomingBites[lastIndex], 0, sizeof(SoundBite));
//		m_timestampForCurrentIncoming += BITE_LENGTH;
//		Sound* sound = m_incomingSoundBites[/*m_currentBiteIndex*/0];
//		The.Audio->PutDataInSound(m_incomingBites[m_currentBiteIndex].buffer, BYTES_PER_BITE, sound);
//		The.Audio->PlaySound(sound);
//		int advanceIndex = m_currentBiteIndex;
//		IncrementIndexWrapped(advanceIndex, BUFFERED_BITE_COUNT);
//		m_currentBiteIndex = (byte)advanceIndex;
//	}
//}
//
//
////-----------------------------------------------------------------------------------------------
//void VoiceChatSystem::SetSourceTime(float sourceTime)
//{
//	//Now we can add the converter to the current time to get source time or subtract converter from source to get to our time
//	m_timeConverter = sourceTime - m_currentTime;
//}
//
//
////-----------------------------------------------------------------------------------------------
//ushort VoiceChatSystem::HandleMessage(NetMessage& msg)
//{
//	float srcTimeStamp;
//	ushort biteIndex;
//	size_t indexIntoChunk;
//	size_t chunksInBite;
//	msg.Read<float>(srcTimeStamp);
//	msg.Read<ushort>(biteIndex);
//	msg.Read<size_t>(indexIntoChunk);
//	msg.Read<size_t>(chunksInBite);
//
//	//Get correct bite for this message
//	SoundBite* toPopulate = GetBiteForTimestamp(srcTimeStamp, biteIndex);
//	if (!toPopulate)
//	{
//		return 65535;
//	}
//	toPopulate->numChunksReceived++;
//	ushort numBytes;
//	char buffer[LARGE_MESSAGE_CHUNK_BYTES];
//	msg.ReadBuffer(buffer, numBytes);
//
//	memcpy(toPopulate->buffer + indexIntoChunk * LARGE_MESSAGE_CHUNK_BYTES, buffer, numBytes);
//
//	return biteIndex;
//}
//
//
////-----------------------------------------------------------------------------------------------
//SoundBite* VoiceChatSystem::GetBiteForTimestamp(float timestamp, ushort biteIndex, bool fromSource /* = true */)
//{
//	float myTimestamp = timestamp;
//	if (fromSource)
//	{
//		myTimestamp = myTimestamp - m_timeConverter;
//	}
//	
//	float readHead = m_currentTime - CLIENT_DELAY_FROM_INIT;
//
//	if (myTimestamp <= readHead - POPULATE_NEXT_BUFFER_DEADLINE)
//	{
//		return nullptr;
//	}
//	
//	return &m_incomingBites[biteIndex % BUFFERED_BITE_COUNT];
//}
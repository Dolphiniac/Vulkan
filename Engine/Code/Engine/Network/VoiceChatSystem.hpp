#pragma once
//
//#include "Engine/Network/NetworkSystem.hpp"
//#include "Engine/Core/ObjectPool.hpp"
//#include "Engine/Core/Audio.hpp"
//#include "Engine/Core/EventSystem.hpp"
//
//#define SAMPLES_PER_BITE 22050	//Sample rate of 44100 in .5 seconds
//#define BYTES_PER_SAMPLE 2
//#define NUM_MICROPHONES 2
//#define BYTES_PER_BITE SAMPLES_PER_BITE * BYTES_PER_SAMPLE * NUM_MICROPHONES
//
//#define BUFFERED_BITE_COUNT 20
//#define CLIENT_DELAY_FROM_INIT 5.f
//#define POPULATE_NEXT_BUFFER_DEADLINE .2f
//#define BITE_LENGTH .5f
//
////-----------------------------------------------------------------------------------------------
//struct SoundBite
//{
//	char buffer[BYTES_PER_BITE];
//	float timestamp;
//	ushort numChunksReceived;
//};
//
//
////-----------------------------------------------------------------------------------------------
//struct SoundBiteData : Event
//{
//	SoundBite* soundbite;
//	ushort biteIndex;
//};
//
//
////-----------------------------------------------------------------------------------------------
//class VoiceChatSystem
//{
//public:
//	VoiceChatSystem();
//	~VoiceChatSystem();
//	void Initialize();
//	void Tick(struct Event* e);
//	float GetTime() const { return m_currentTime; }
//	void SetSourceTime(float sourceTime);
//	ushort HandleMessage(class NetMessage& msg);
//
//private:
//	SoundBite* GetBiteForTimestamp(float timestamp, ushort biteIndex, bool fromSource = true);
//
//private:
//	SoundBite m_incomingBites[BUFFERED_BITE_COUNT];
//	Sound* m_incomingSoundBites[BUFFERED_BITE_COUNT];
//	Sound* m_recordingSound;
//	float m_timeConverter;
//	float m_currentTime;
//	float m_timeOnCurrentBite;
//	byte m_currentBiteIndex;
//	float m_timestampForCurrentOutgoing;
//	float m_timestampForCurrentIncoming;
//	ushort m_currentOutgoingBiteIndex;
//	float m_currentIncomingOffset;
//};
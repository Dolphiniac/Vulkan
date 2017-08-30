#if !defined(INCLUDED_AUDIO)
#define INCLUDED_AUDIO
#pragma once

////---------------------------------------------------------------------------
//#pragma comment( lib, "ThirdParty/fmod/fmodex_vc" ) // Link in the fmodex_vc.lib static library
//
////---------------------------------------------------------------------------
//#include "ThirdParty/fmod/fmod.hpp"
//#include <string>
//#include <vector>
//#include <map>
//#undef PlaySound
//
////---------------------------------------------------------------------------
//typedef unsigned int SoundID;
//typedef FMOD::Channel* AudioChannelHandle;
//typedef FMOD::Sound Sound;
//const unsigned int MISSING_SOUND_ID = 0xffffffff;
//
//
///////////////////////////////////////////////////////////////////////////////
//class AudioSystem
//{
//	friend class EngineSystemManager;
//public:
//	virtual ~AudioSystem();
//	SoundID CreateOrGetSound( const std::string& soundFileName );
//	AudioChannelHandle PlaySound( SoundID soundID, float volumeLevel=1.f );
//	void Stop(AudioChannelHandle channelToStop);
//	void Tick( float deltaSeconds ); // Must be called at regular intervals (e.g. every frame)
//	AudioSystem();
//	FMOD::Sound* GetRecordingSound(size_t numBytes);
//	FMOD::Sound* CreateFromBuffer(void* buffer, unsigned int numBytes);
//	AudioChannelHandle PlaySound(FMOD::Sound* toPlay);
//	void StartRecord(FMOD::Sound* toRecord);
//	void StopRecord();
//
//	//The below functions are helpers for copying data to and from sounds, assuming correct buffer size
//	void GetDataFromSound(void* outBuffer, size_t numBytes, Sound* sound);
//	void PutDataInSound(const void* buffer, size_t numBytes, Sound* sound);
//
//protected:
//	void InitializeFMOD();
//	void ValidateResult( FMOD_RESULT result );
//
//protected:
//	FMOD::System*						m_fmodSystem;
//	std::map< std::string, SoundID >	m_registeredSoundIDs;
//	std::vector< FMOD::Sound* >			m_registeredSounds;
//
//private:
//};
//
//
////---------------------------------------------------------------------------
//void InitializeAudio();
//
//
#endif // INCLUDED_AUDIO

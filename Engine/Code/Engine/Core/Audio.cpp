////---------------------------------------------------------------------------
//#include "Audio.hpp"
////#include "EngineCommon.hpp"
//
//
////---------------------------------------------------------------------------
//AudioSystem::AudioSystem()
//	: m_fmodSystem( nullptr )
//	, m_registeredSoundIDs()
//{
//	InitializeFMOD();
//}
//
//
////---------------------------------------------------------------------------
//// FMOD startup code based on "GETTING STARTED With FMOD Ex Programmer’s API for Windows" document
////	from the FMOD programming API at http://www.fmod.org/download/
////
//void AudioSystem::InitializeFMOD()
//{
//	const int MAX_AUDIO_DEVICE_NAME_LEN = 256;
//	FMOD_RESULT result;
//	unsigned int fmodVersion;
//	int numDrivers;
//	FMOD_SPEAKERMODE speakerMode;
//	FMOD_CAPS deviceCapabilities;
//	char audioDeviceName[ MAX_AUDIO_DEVICE_NAME_LEN ];
//
//	// Create a System object and initialize.
//	result = FMOD::System_Create( &m_fmodSystem );
//	ValidateResult( result );
//
//	result = m_fmodSystem->getVersion( &fmodVersion );
//	ValidateResult( result );
//
//	if( fmodVersion < FMOD_VERSION )
//	{
//		//DebuggerPrintf( "AUDIO SYSTEM ERROR!  Your FMOD .dll is of an older version (0x%08x == %d) than that the .lib used to compile this code (0x%08x == %d).\n", fmodVersion, fmodVersion, FMOD_VERSION, FMOD_VERSION );
//	}
//
//	result = m_fmodSystem->getNumDrivers( &numDrivers );
//	ValidateResult( result );
//
//	if( numDrivers == 0 )
//	{
//		result = m_fmodSystem->setOutput( FMOD_OUTPUTTYPE_NOSOUND );
//		ValidateResult( result );
//	}
//	else
//	{
//		result = m_fmodSystem->getDriverCaps( 0, &deviceCapabilities, 0, &speakerMode );
//		ValidateResult( result );
//
//		// Set the user selected speaker mode.
//		result = m_fmodSystem->setSpeakerMode( speakerMode );
//		ValidateResult( result );
//
//		if( deviceCapabilities & FMOD_CAPS_HARDWARE_EMULATED )
//		{
//			// The user has the 'Acceleration' slider set to off! This is really bad
//			// for latency! You might want to warn the user about this.
//			result = m_fmodSystem->setDSPBufferSize( 1024, 10 );
//			ValidateResult( result );
//		}
//
//		result = m_fmodSystem->getDriverInfo( 0, audioDeviceName, MAX_AUDIO_DEVICE_NAME_LEN, 0 );
//		ValidateResult( result );
//
//		if( strstr( audioDeviceName, "SigmaTel" ) )
//		{
//			// Sigmatel sound devices crackle for some reason if the format is PCM 16bit.
//			// PCM floating point output seems to solve it.
//			result = m_fmodSystem->setSoftwareFormat( 48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0, FMOD_DSP_RESAMPLER_LINEAR );
//			ValidateResult( result );
//		}
//	}
//
//	result = m_fmodSystem->init( 100, FMOD_INIT_NORMAL, 0 );
//	if( result == FMOD_ERR_OUTPUT_CREATEBUFFER )
//	{
//		// Ok, the speaker mode selected isn't supported by this sound card. Switch it
//		// back to stereo...
//		result = m_fmodSystem->setSpeakerMode( FMOD_SPEAKERMODE_STEREO );
//		ValidateResult( result );
//
//		// ... and re-init.
//		result = m_fmodSystem->init( 100, FMOD_INIT_NORMAL, 0 );
//		ValidateResult( result );
//	}
//}
//
//
////---------------------------------------------------------------------------
//AudioSystem::~AudioSystem()
//{
// 	FMOD_RESULT result = FMOD_OK;
//	result = m_fmodSystem->close();//FMOD_System_Close( m_fmodSystem );
//	result = m_fmodSystem->release();//FMOD_System_Release( m_fmodSystem );
//	m_fmodSystem = nullptr;
//}
//
//
////---------------------------------------------------------------------------
//SoundID AudioSystem::CreateOrGetSound( const std::string& soundFileName )
//{
//	std::map< std::string, SoundID >::iterator found = m_registeredSoundIDs.find( soundFileName );
//	if( found != m_registeredSoundIDs.end() )
//	{
//		return found->second;
//	}
//	else
//	{
//		FMOD::Sound* newSound = nullptr;
//		m_fmodSystem->createSound( soundFileName.c_str(), FMOD_DEFAULT, nullptr, &newSound );
//		if( newSound )
//		{
//			SoundID newSoundID = m_registeredSounds.size();
//			m_registeredSoundIDs[ soundFileName ] = newSoundID;
//			m_registeredSounds.push_back( newSound );
//			return newSoundID;
//		}
//	}
//
//	return MISSING_SOUND_ID;
//}
//
//
////---------------------------------------------------------------------------
//FMOD::Channel* AudioSystem::PlaySound( SoundID soundID, float volumeLevel )
//{
//	unsigned int numSounds = m_registeredSounds.size();
//	if( soundID < 0 || soundID >= numSounds )
//		return nullptr;
//
//	FMOD::Sound* sound = m_registeredSounds[ soundID ];
//	if( !sound )
//		return nullptr;
//
//	FMOD::Channel* channelAssignedToSound = nullptr;
//	m_fmodSystem->playSound( FMOD_CHANNEL_FREE, sound, false, &channelAssignedToSound );
//	if( channelAssignedToSound )
//	{
//		channelAssignedToSound->setVolume( volumeLevel );
//	}
//	return channelAssignedToSound;
//}
//
//
////-----------------------------------------------------------------------------------------------
//void AudioSystem::Stop(AudioChannelHandle channelToStop)
//{
//	channelToStop->stop();
//}
//
//
////---------------------------------------------------------------------------
//void AudioSystem::Tick( float deltaSeconds )
//{
//	deltaSeconds += 3.f; //Suppress "unused variable" warning
//	FMOD_RESULT result = m_fmodSystem->update();
//	ValidateResult( result );
//}
//
//
////-----------------------------------------------------------------------------------------------
//FMOD::Sound* AudioSystem::GetRecordingSound(size_t numBytes)
//{
//	FMOD_CREATESOUNDEXINFO info;
//	memset(&info, 0, sizeof(FMOD_CREATESOUNDEXINFO));
//	info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
//	info.numchannels = 2;
//	info.format = FMOD_SOUND_FORMAT_PCM16;
//	info.defaultfrequency = 44100;
//	info.length = numBytes;
//
//	FMOD::Sound* result = nullptr;
//	m_fmodSystem->createSound(0, FMOD_LOOP_NORMAL | FMOD_OPENUSER, &info, &result);
//
//	result->setLoopCount(0);
//	return result;
//}
//
//
////-----------------------------------------------------------------------------------------------
//FMOD::Sound* AudioSystem::CreateFromBuffer(void* buffer, unsigned int numBytes)
//{
//	FMOD_CREATESOUNDEXINFO info;
//	memset(&info, 0, sizeof(FMOD_CREATESOUNDEXINFO));
//	info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
//	info.numchannels = 2;
//	info.format = FMOD_SOUND_FORMAT_PCM16;
//	info.defaultfrequency = 44100;
//	info.length = numBytes;
//
//	FMOD::Sound* result = nullptr;
//	m_fmodSystem->createSound(0, FMOD_LOOP_NORMAL | FMOD_OPENUSER, &info, &result);
//
//	void* lock1;
//	void* lock2;
//	unsigned int len1;
//	unsigned int len2;
//	result->lock(0, numBytes, &lock1, &lock2, &len1, &len2);
//	memcpy(lock1, buffer, numBytes);
//	result->unlock(lock1, lock2, len1, len2);
//
//	return result;
//}
//
//
////-----------------------------------------------------------------------------------------------
//AudioChannelHandle AudioSystem::PlaySound(FMOD::Sound* toPlay)
//{
//	AudioChannelHandle handle = nullptr;
//	m_fmodSystem->playSound(FMOD_CHANNEL_FREE, toPlay, false, &handle);
//	return handle;
//}
//
//
////-----------------------------------------------------------------------------------------------
//void AudioSystem::StartRecord(FMOD::Sound* toRecord)
//{
//	m_fmodSystem->recordStart(0, toRecord, false);
//}
//
//
////-----------------------------------------------------------------------------------------------
//void AudioSystem::StopRecord()
//{
//	m_fmodSystem->recordStop(0);
//}
//
//
////-----------------------------------------------------------------------------------------------
//void AudioSystem::GetDataFromSound(void* outBuffer, size_t numBytes, Sound* sound)
//{
//	void* ptr1;
//	void* ptr2;
//	unsigned int len1;
//	unsigned int len2;
//
//	sound->lock(0, numBytes, &ptr1, &ptr2, &len1, &len2);
//	memcpy(outBuffer, ptr1, numBytes);
//	sound->unlock(ptr1, ptr2, len1, len2);
//}
//
//
////-----------------------------------------------------------------------------------------------
//void AudioSystem::PutDataInSound(const void* buffer, size_t numBytes, Sound* sound)
//{
//	void* ptr1;
//	void* ptr2;
//	unsigned int len1;
//	unsigned int len2;
//
//	sound->lock(0, numBytes, &ptr1, &ptr2, &len1, &len2);
//	memcpy(ptr1, buffer, numBytes);
//	sound->unlock(ptr1, ptr2, len1, len2);
//}
//
//
////---------------------------------------------------------------------------
//void AudioSystem::ValidateResult( FMOD_RESULT result )
//{
//	if( result != FMOD_OK )
//	{
//		//DebuggerPrintf( "AUDIO SYSTEM ERROR: Got error result code %d.\n", result );
//		__debugbreak();
//	}
//}

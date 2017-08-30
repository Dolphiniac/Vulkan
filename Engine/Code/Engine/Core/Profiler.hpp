#pragma once

#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Core/ObjectPool.hpp"
#include "Engine/Core/Logger.hpp"
#include <string>


//-----------------------------------------------------------------------------------------------
typedef unsigned __int64 uint64_t;


//-----------------------------------------------------------------------------------------------
enum EProfileReportFormat
{
	PRF_FLAT_VIEW,
	PRF_HIERARCHICAL_VIEW
};


//-----------------------------------------------------------------------------------------------
struct ProfileSample
{
	const char* tag;
	uint64_t startCounter;
	uint64_t endCounter;
	ProfileSample* parent;
	ProfileSample* firstChild;
	ProfileSample* nextSibling;
	uint64_t bytesAllocated;
	uint64_t bytesFreed;

	//Initialize with tag, make sure pointers are null until set
	ProfileSample(const char* thisTag) : tag(thisTag), parent(nullptr), firstChild(nullptr), nextSibling(nullptr), bytesAllocated(0), bytesFreed(0) {}
};


//-----------------------------------------------------------------------------------------------
class Profiler
{
public:
	static void Startup();
	static void Shutdown();
	static void MarkFrame();
	static void PushSample(const char* tag);
	static void PopSample();
	static bool ToggleProfiling();
	static ProfileSample* GetLastFrame();
	static std::string CompileReport(EProfileReportFormat format);
	static void AddAllocation(const uint64_t& bytesAllocated);
	static void SubtractAllocation(const uint64_t& bytesFreed);
	static bool IsProfiling();

private:
	static void RecursivelyFreeSample(ProfileSample* toDelete);

private:
	static bool s_isProfiling;
	static bool s_desiresProfiling;
	static ProfileSample* s_currFrame;
	static ProfileSample* s_prevFrame;
	static ProfileSample* s_currSample;
	static ObjectPool<ProfileSample>* s_samples;
};


//-----------------------------------------------------------------------------------------------
namespace ProfilerHelper
{
	uint64_t GetCurrentPerformanceCounter();
	double PerformanceCountToSeconds(const uint64_t& performanceCounter);
	double GetCurrentSeconds();

	//RAII profile sample for measuring function performance (scoped)
	class RAIISample
	{
	public:
		RAIISample(const char* tag)
		{
			Profiler::PushSample(tag);
		}
		~RAIISample()
		{
			Profiler::PopSample();
		}
	};

	//Used in conjunction with logger to get a quick and dirty sample (scoped)
	class RAIISampleDirty
	{
	public:
		RAIISampleDirty(const char* tag)
		{
			//Using malloc and free so that you don't have to have the profiler set up.  You do need the logger, though
			m_mySample = (ProfileSample*)malloc(sizeof(ProfileSample));
			m_mySample->tag = tag;
			m_mySample->startCounter = GetCurrentPerformanceCounter();
		}
		~RAIISampleDirty()
		{
			m_mySample->endCounter = GetCurrentPerformanceCounter();
			uint64_t timelapse = m_mySample->endCounter - m_mySample->startCounter;
			double ms = PerformanceCountToSeconds(timelapse) * 1000.;
			Logger::Printf("%s    %llu cycles    %.3f ms", m_mySample->tag, timelapse, ms);
			Logger::Flush();
			free(m_mySample);
		}

	private:
		ProfileSample* m_mySample;
	};

}

//Normal scoped profile section; plays nicely with rest of profiler
#define PROFILE_LOG_SECTION(tag) ProfilerHelper::RAIISample scopedSample(#tag);

//Dirty scoped profile section; interfaces with logger for quick measurement of performance
#define PROFILE_LOG_SECTION_DIRTY(tag) ProfilerHelper::RAIISampleDirty scopedSampleDirty(#tag);


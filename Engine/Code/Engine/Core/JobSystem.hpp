#pragma once

#include "Engine/Memory/ThreadSafeSTL.hpp"
#include "Engine/Core/LinearAllocator.hpp"

#include <vector>
#include <thread>
#include <map>


//-----------------------------------------------------------------------------------------------
enum EJobCategory
{
	GENERIC = 1,
	GENERIC_SLOW = 1 << 1
};
#define NUM_JOB_CATEGORIES 2
//IMPORTANT!!!! UPDATE DEFINE WHENEVER THE CATEGORY NUM CHANGES!!!!!


//-----------------------------------------------------------------------------------------------
typedef void(*JobWorkFunc)(class Job*);

enum EJobState
{
	JOB_STATE_SPAWNED,
	JOB_STATE_ENQUEUED,
	JOB_STATE_IN_PROGRESS,
	JOB_STATE_COMPLETE
};


//-----------------------------------------------------------------------------------------------
class Job
{
	const size_t ALLOCATOR_BYTES = 64;

public:
	static Job* Create(EJobCategory type, JobWorkFunc workFunc);

	//Don't call this explicitly
	Job() : m_jobMemory(ALLOCATOR_BYTES), m_currentOffset(0), m_currState(JOB_STATE_SPAWNED) {}
	
	static void Dispatch(Job* toDispatch);

	template<typename T>
	void Read(T& outVal)
	{
		memcpy(&outVal, m_jobMemory.Get(m_currentOffset), sizeof(T));
		m_currentOffset += sizeof(T);
	}

	template<typename T>
	void Write(const T& inVal)
	{
		memcpy(m_jobMemory.Get(m_currentOffset), &inVal, sizeof(T));
		m_currentOffset += sizeof(T);
	}

public:
	JobWorkFunc DoWork;
	EJobState m_currState;

private:
	LinearAllocator m_jobMemory;
	size_t m_currentOffset;
	EJobCategory m_category;
};


//-----------------------------------------------------------------------------------------------
class JobConsumer
{
public:
	JobConsumer(EJobCategory priority, unsigned int categoryMask) : m_priority(priority), m_categoryMask(categoryMask) {}
	void ConsumeAllJobs();
	bool ConsumeJob();

private:
	EJobCategory m_priority;
	unsigned int m_categoryMask;
};


//-----------------------------------------------------------------------------------------------
namespace JobSystem
{
	//Pass in categories using a bitwise OR (e.g. GENERIC | GENERIC_SLOW)
	void Startup(unsigned int categoryMask, int numThreads);
	void Shutdown();
	void JobWorkerExecute(EJobCategory priority, unsigned int allSupportedCategories);
	void WaitOnJobs(Job** toWait, size_t numJobs = 1);
	void DetachJobs(Job** toDetach, size_t numJobs = 1);

	extern std::vector<std::thread*> g_threadHandles;
	extern bool g_shouldShutDown;
	extern std::map<EJobCategory, ThreadSafeQueue<Job*>> g_jobQueues;
};
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/ReferenceCount.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//-----------------------------------------------------------------------------------------------
std::vector<std::thread*> JobSystem::g_threadHandles;
bool JobSystem::g_shouldShutDown = false;
std::map<EJobCategory, ThreadSafeQueue<Job*>> JobSystem::g_jobQueues;


//-----------------------------------------------------------------------------------------------
void JobSystem::Startup(unsigned int categoryMask, int numThreads)
{
	ASSERT_OR_DIE(categoryMask != 0, "Cannot specify no categories");
	int threadsToSpawn = numThreads;
	if (numThreads < 0)
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		threadsToSpawn = (int)si.dwNumberOfProcessors + numThreads;
	}
	if (threadsToSpawn <= 0)
	{
		threadsToSpawn = 1;
	}
	int currentCategoryCount = 0;
	for (int i = 0; i < NUM_JOB_CATEGORIES; i++)
	{
		EJobCategory category = (EJobCategory)(1 << i);
		g_jobQueues.insert(std::make_pair(category, ThreadSafeQueue<Job*>()));
	}
	for (int i = 0; i < threadsToSpawn; i++)
	{
		unsigned int thisCategory = 1 << currentCategoryCount;
		//Always support generic and generic slow, but don't necessarily prioritize
		unsigned int allConsumerCategories = thisCategory | GENERIC | GENERIC_SLOW;
		std::thread* currThread = new std::thread(JobWorkerExecute, (EJobCategory)thisCategory, allConsumerCategories);
		g_threadHandles.push_back(currThread);
		for (;;)
		{
			++currentCategoryCount;
			if (currentCategoryCount >= NUM_JOB_CATEGORIES)
			{
				currentCategoryCount = 0;
			}
			
			if (((1 << currentCategoryCount) & categoryMask) != 0)
			{
				break;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void JobSystem::Shutdown()
{
	g_shouldShutDown = true;
	for (std::thread* t : g_threadHandles)
	{
		t->join();
		delete t;
	}
	g_threadHandles.clear();
	g_threadHandles.shrink_to_fit();

	//Clean up any remaining jobs, just in case
	unsigned int allCategories = ~0U;
	JobConsumer consumer(GENERIC, allCategories);
	consumer.ConsumeAllJobs();
	g_jobQueues.clear();
}


//-----------------------------------------------------------------------------------------------
void JobSystem::JobWorkerExecute(EJobCategory priority, unsigned int allSupportedCategories)
{
	JobConsumer consumer(priority, allSupportedCategories);

	while (!g_shouldShutDown)
	{
		consumer.ConsumeAllJobs();
		std::this_thread::yield();
	}

	//Consume remaining jobs
	consumer.ConsumeAllJobs();
}


//-----------------------------------------------------------------------------------------------
void JobConsumer::ConsumeAllJobs()
{
	while (ConsumeJob());
}


//-----------------------------------------------------------------------------------------------
static bool FinishJob(Job* toFinish)
{
	//Try to grab the job.  If success, do the work.  Otherwise, someone else got it, so give up
	if ((EJobCategory)InterlockedCompareExchange((volatile unsigned int*)&toFinish->m_currState, JOB_STATE_IN_PROGRESS, JOB_STATE_ENQUEUED) == JOB_STATE_ENQUEUED)
	{
		toFinish->DoWork(toFinish);
		toFinish->m_currState = JOB_STATE_COMPLETE;

		//Releases the original reference, making the consumer the de facto owner
		RefCount::Release<Job>(toFinish);

		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool JobConsumer::ConsumeJob()
{
	Job* currJob = nullptr;
	for (;;)
	{
		if (JobSystem::g_jobQueues.at(m_priority).Dequeue(&currJob))
		{
			if (FinishJob(currJob))
			{
				return true;
			}
			else
			{
				//The dispatcher got this job.  Get another from the queue
				continue;
			}
		}
		else
		{
			//This queue was empty.  Stop trying
			break;
		}
	}

	for (int i = 0; i < NUM_JOB_CATEGORIES; i++)
	{
		unsigned int currMask = (1 << i);
		if ((currMask & m_categoryMask) != 0)
		{
			EJobCategory currCategory = (EJobCategory)currMask;
			for (;;)
			{
				if (JobSystem::g_jobQueues.at(currCategory).Dequeue(&currJob))
				{
					if (FinishJob(currJob))
					{
						return true;
					}
					else
					{
						continue;
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
Job* Job::Create(EJobCategory type, JobWorkFunc workFunc)
{
	Job* result = RefCount::CreateAndAcquire<Job>();
	result->DoWork = workFunc;
	result->m_category = type;

	return result;
}


//-----------------------------------------------------------------------------------------------
void Job::Dispatch(Job* toDispatch)
{
	RefCount::Acquire(toDispatch);

	//Reset offset into allocator to rewind and read in the same order we wrote
	toDispatch->m_currentOffset = 0;

	toDispatch->m_currState = JOB_STATE_ENQUEUED;
	JobSystem::g_jobQueues.at(toDispatch->m_category).Enqueue(toDispatch);
}


//-----------------------------------------------------------------------------------------------
void JobSystem::WaitOnJobs(Job** toWait, size_t numJobs)
{
	unsigned int allCategories = ~0U;
	JobConsumer consumer(GENERIC, allCategories);

	for (size_t i = 0; i < numJobs; i++)
	{
		Job* currJob = toWait[i];
		while (currJob->m_currState != JOB_STATE_COMPLETE)
		{
			//Try to do this job.  If you fail, someone else is doing it, so consume another job
			if (FinishJob(currJob))
			{
				break;
			}

			consumer.ConsumeJob();
		}

		RefCount::Release<Job>(currJob);
	}
}


//-----------------------------------------------------------------------------------------------
void JobSystem::DetachJobs(Job** toDetach, size_t numJobs)
{
	for (size_t i = 0; i < numJobs; i++)
	{
		RefCount::Release<Job>(toDetach[i]);
	}
}
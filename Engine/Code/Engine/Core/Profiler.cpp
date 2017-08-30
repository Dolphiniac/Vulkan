#include "Engine/Core/Profiler.hpp"
#include "Engine/Core/StringUtils.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//-----------------------------------------------------------------------------------------------
// PROFILER_HELPER NAMESPACE FUNCTIONS
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
uint64_t ProfilerHelper::GetCurrentPerformanceCounter()
{
	LARGE_INTEGER result;
	QueryPerformanceCounter(&result);
	return result.QuadPart;
}


//-----------------------------------------------------------------------------------------------
double ProfilerHelper::PerformanceCountToSeconds(const uint64_t& performanceCounter)
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	double secondsPerCount = 1. / (double)frequency.QuadPart;
	return secondsPerCount * (double)performanceCounter;
}


//-----------------------------------------------------------------------------------------------
double ProfilerHelper::GetCurrentSeconds()
{
	return PerformanceCountToSeconds(GetCurrentPerformanceCounter());
}


//-----------------------------------------------------------------------------------------------
// END PROFILER_HELPER NAMESPACE FUNCTIONS
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
bool Profiler::s_isProfiling = false;
bool Profiler::s_desiresProfiling = false;
ProfileSample* Profiler::s_currFrame = nullptr;
ProfileSample* Profiler::s_prevFrame = nullptr;
ProfileSample* Profiler::s_currSample = nullptr;
ObjectPool<ProfileSample>* Profiler::s_samples = nullptr;


//-----------------------------------------------------------------------------------------------
#ifdef __USING_PROFILER
//-----------------------------------------------------------------------------------------------
void Profiler::Startup()
{
	s_desiresProfiling = true;
	s_samples = new ObjectPool<ProfileSample>(1024);
}


//-----------------------------------------------------------------------------------------------
void Profiler::Shutdown()
{
	s_desiresProfiling = false;
	s_isProfiling = false;
	delete s_samples;
}


//-----------------------------------------------------------------------------------------------
void Profiler::MarkFrame()
{
	if (s_isProfiling)
	{
		PopSample();
		ASSERT_OR_DIE(!s_currSample, "Leaked profile samples!");
		if (s_prevFrame)
		{
			RecursivelyFreeSample(s_prevFrame);
		}
		s_prevFrame = s_currFrame;
		s_currFrame = nullptr;
	}

	s_isProfiling = s_desiresProfiling;

	if (s_isProfiling)
	{
		PushSample("frame");
	}
}


//-----------------------------------------------------------------------------------------------
void Profiler::PushSample(const char* tag)
{
	if (!s_isProfiling)
	{
		return;
	}

	ProfileSample* currSample = s_samples->Create(tag);
	currSample->startCounter = ProfilerHelper::GetCurrentPerformanceCounter();

	if (s_currSample)
	{
		currSample->parent = s_currSample;

		//Is it the first child?
		if (!s_currSample->firstChild)
		{
			s_currSample->firstChild = currSample;
		}
		else
		{
			ProfileSample* currChild = s_currSample->firstChild;
			while (currChild->nextSibling)
			{
				currChild = currChild->nextSibling;
			}
			currChild->nextSibling = currSample;
		}
	}
	else
	{
		s_currFrame = currSample;
	}

	s_currSample = currSample;
}


//-----------------------------------------------------------------------------------------------
void Profiler::PopSample()
{
	if (!s_isProfiling)
	{
		return;
	}

	ASSERT_OR_DIE(s_currSample, "Attempted to pop null sample");
	s_currSample->endCounter = ProfilerHelper::GetCurrentPerformanceCounter();
	s_currSample = s_currSample->parent;
}


//-----------------------------------------------------------------------------------------------
bool Profiler::ToggleProfiling()
{
	s_desiresProfiling = !s_isProfiling;
	return s_desiresProfiling;
}


//-----------------------------------------------------------------------------------------------
ProfileSample* Profiler::GetLastFrame()
{
	return s_prevFrame;
}


//-----------------------------------------------------------------------------------------------
void Profiler::RecursivelyFreeSample(ProfileSample* toDelete)
{
	if (!toDelete)
	{
		return;
	}

	RecursivelyFreeSample(toDelete->firstChild);
	RecursivelyFreeSample(toDelete->nextSibling);

	s_samples->Free(toDelete);
}

struct ReportEntry
{
	const char* tag;
	uint64_t totalCycles;
	uint64_t selfCycles;
	ReportEntry* firstChild;
	ReportEntry* nextSibling;
	unsigned int calls;
	uint64_t bytesAllocated;
	uint64_t bytesFreed;

	ReportEntry() : firstChild(nullptr), nextSibling(nullptr), calls(1), bytesAllocated(0), bytesFreed(0) {}
};


//-----------------------------------------------------------------------------------------------
static ReportEntry* CreateEntryAndAddToTreeRecursively(const ProfileSample* currSample, ReportEntry* parentEntry, ObjectPool<ReportEntry>* entries)
{
	if (!currSample)
	{
		return nullptr;
	}
	ReportEntry* result = entries->Create();
	result->tag = currSample->tag;
	result->totalCycles = currSample->endCounter - currSample->startCounter;
	result->selfCycles = result->totalCycles;
	result->bytesAllocated = currSample->bytesAllocated;
	result->bytesFreed = currSample->bytesFreed;

	if (parentEntry)
	{
		parentEntry->selfCycles -= result->totalCycles;
	}

	result->firstChild = CreateEntryAndAddToTreeRecursively(currSample->firstChild, result, entries);
	result->nextSibling = CreateEntryAndAddToTreeRecursively(currSample->nextSibling, parentEntry, entries);

	return result;
}


//-----------------------------------------------------------------------------------------------
static void MakeAllNodesSiblingsOfRootRecursively(ReportEntry* currEntry, ReportEntry*& lastSiblingOfRoot, bool isRoot)
{
	if (!currEntry)
	{
		return;
	}

	MakeAllNodesSiblingsOfRootRecursively(currEntry->firstChild, lastSiblingOfRoot, false);
	currEntry->firstChild = nullptr;

	if (!isRoot)
	{
		MakeAllNodesSiblingsOfRootRecursively(currEntry->nextSibling, lastSiblingOfRoot, false);
		currEntry->nextSibling = nullptr;
		lastSiblingOfRoot->nextSibling = currEntry;
		lastSiblingOfRoot = currEntry;
	}
}


//-----------------------------------------------------------------------------------------------
static void ConcatenateSiblingsInTreeRecursively(ReportEntry* currEntry, ObjectPool<ReportEntry>* entries)
{
	if (!currEntry)
	{
		return;
	}

	ReportEntry* checkEntry = currEntry;
	ReportEntry* lastChild = currEntry->firstChild;
	if (lastChild)
	{
		while (lastChild->nextSibling)
		{
			lastChild = lastChild->nextSibling;
		}
	}

	while (checkEntry->nextSibling)
	{
		//If this sibling has my tag, steal its cycles and children, and remove it
		if (checkEntry->nextSibling->tag == currEntry->tag)
		{
			ReportEntry* toFree = checkEntry->nextSibling;
			currEntry->totalCycles += toFree->totalCycles;
			currEntry->selfCycles += toFree->selfCycles;
			currEntry->calls += toFree->calls;
			currEntry->bytesAllocated += toFree->bytesAllocated;
			currEntry->bytesFreed += toFree->bytesFreed;

			if (lastChild)
			{
				lastChild->nextSibling = toFree->firstChild;
			}
			else
			{
				lastChild = toFree->firstChild;
			}
			if (lastChild)
			{
				while (lastChild->nextSibling)
				{
					lastChild = lastChild->nextSibling;
				}
			}

			checkEntry->nextSibling = toFree->nextSibling;
			entries->Free(toFree);
		}

		if (checkEntry->nextSibling)
		{
			checkEntry = checkEntry->nextSibling;
		}
	}

	//Do this for each of my children and siblings recursively
	ConcatenateSiblingsInTreeRecursively(currEntry->firstChild, entries);
	ConcatenateSiblingsInTreeRecursively(currEntry->nextSibling, entries);
}


//-----------------------------------------------------------------------------------------------
static std::string GetStringForEntryRecursively(ReportEntry* currEntry, uint64_t frameDuration, int level)
{
	std::string myString = "";
	if (!currEntry)
	{
		return myString;
	}

	for (int i = 0; i < level; i++)
	{
		myString.push_back('-');
	}
	myString += currEntry->tag;
	myString.push_back('\t');
	myString += std::to_string(currEntry->calls);
	myString += "\t\t";
	myString += Stringf("%llu Cycles (%.3fms)", currEntry->totalCycles, ProfilerHelper::PerformanceCountToSeconds(currEntry->totalCycles) * 1000.);
	myString.push_back('\t');
	myString += Stringf("%llu Cycles (%.3fms)", currEntry->selfCycles, ProfilerHelper::PerformanceCountToSeconds(currEntry->selfCycles) * 1000.);
	myString.push_back('\t');
	myString += Stringf("%.2f%%", (double)currEntry->totalCycles / (double)frameDuration * 100.);
	myString.push_back('\t');
	myString += Stringf("%llu\t%llu\n", currEntry->bytesAllocated, currEntry->bytesFreed);

	myString += GetStringForEntryRecursively(currEntry->firstChild, frameDuration, level + 1);
	myString += GetStringForEntryRecursively(currEntry->nextSibling, frameDuration, level);

	return myString;
}


//-----------------------------------------------------------------------------------------------
std::string Profiler::CompileReport(EProfileReportFormat format)
{
	if (!s_prevFrame)
	{
		return "";
	}

	uint64_t frameDuration = s_prevFrame->endCounter - s_prevFrame->startCounter;

	ObjectPool<ReportEntry>* entries = new ObjectPool<ReportEntry>(1024);
	ReportEntry* rootEntry = CreateEntryAndAddToTreeRecursively(s_prevFrame, nullptr, entries);
	ReportEntry* lastSiblingOfRoot = rootEntry;

	if (format == PRF_FLAT_VIEW)
	{
		//If all nodes are on the same level, there is no hierarchy (effectively flat)
		MakeAllNodesSiblingsOfRootRecursively(rootEntry, lastSiblingOfRoot, true);
	}

	ConcatenateSiblingsInTreeRecursively(rootEntry, entries);

	std::string result = "TAG\t\tCALLS\t\tTIME\t\t\t\tSELF_TIME\t\t\tPERCENT_FRAME_TIME\t\tBYTES_ALLOCATED\t\tBYTES_FREED\n";
	result += GetStringForEntryRecursively(rootEntry, frameDuration, 0);

	delete entries;

	return result;
}


//-----------------------------------------------------------------------------------------------
void Profiler::AddAllocation(const uint64_t& bytesAllocated)
{
	if (!s_currSample || !s_isProfiling)
	{
		return;
	}

	s_currSample->bytesAllocated += bytesAllocated;
}


//-----------------------------------------------------------------------------------------------
void Profiler::SubtractAllocation(const uint64_t& bytesFreed)
{
	if (!s_currSample || !s_isProfiling)
	{
		return;
	}

	s_currSample->bytesFreed += bytesFreed;
}


//-----------------------------------------------------------------------------------------------
bool Profiler::IsProfiling()
{
	return s_isProfiling;
}

#else
void Profiler::Startup() {}
void Profiler::Shutdown() {}
void Profiler::MarkFrame() {}
void Profiler::PushSample(const char* tag) { UNUSED(tag); }
void Profiler::PopSample() {}
bool Profiler::ToggleProfiling() { return false; }
void Profiler::RecursivelyFreeSample(ProfileSample* toDelete) { UNUSED(toDelete); }
ProfileSample* Profiler::GetLastFrame() { return nullptr; }
std::string Profiler::CompileReport(EProfileReportFormat format) { UNUSED(format); return ""; }
void Profiler::AddAllocation(const uint64_t& bytesAllocated) { UNUSED(bytesAllocated); }
void Profiler::SubtractAllocation(const uint64_t& bytesFreed) { UNUSED(bytesFreed); }
bool Profiler::IsProfiling() { return false; }
#endif


#include "Engine/Core/ConsoleCommand.hpp"
#include "Engine/Renderer/Rgba.hpp"
#include <sstream>

CONSOLE_COMMAND(ProfilerToggle, args)
{
	UNUSED(args);
	bool willProfile = Profiler::ToggleProfiling();
	ConsolePrintf(WHITE, "Profiling is now %s", (willProfile) ? "ON" : "OFF");
}

CONSOLE_COMMAND(ProfilerPrintFrame, args)
{
	std::string formatString = args.GetNextArg();
	EProfileReportFormat format = PRF_HIERARCHICAL_VIEW;
	if (formatString == "flat")
	{
		format = PRF_FLAT_VIEW;
	}
	else if (formatString == "list" || formatString == "")
	{
		formatString = PRF_HIERARCHICAL_VIEW;
	}
	else
	{
		ConsolePrintf(RED, "Bad argument to profilerprintframe: %s", formatString.c_str());
		return;
	}

	std::string report = Profiler::CompileReport(format);
	std::stringstream ss;
	ss << report;
	std::string currString;
	while (std::getline(ss, currString))
	{
		ConsolePrint(currString, WHITE);
	}
}
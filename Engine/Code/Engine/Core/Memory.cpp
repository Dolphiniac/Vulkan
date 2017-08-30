#include "Engine/Core/Memory.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/callstack.h"
#include "Engine/Core/Logger.hpp"
#include "Engine/Core/Profiler.hpp"
#include "Engine/Core/ConsoleCommand.hpp"

#include <map>


//-----------------------------------------------------------------------------------------------
unsigned int g_numAllocations = 0;
unsigned int g_totalAllocatedBytes = 0;
unsigned int g_numStartupAllocations = 0;
unsigned int g_totalAllocatedBytesStartup = 0;
unsigned int g_currentHighwaterBytes = 0;
unsigned int g_bytesAllocatedSinceLastUpdate = 0;
unsigned int g_bytesFreedSinceLastUpdate = 0;


//-----------------------------------------------------------------------------------------------
static UntrackedMap<void*, callstack_t*> g_callStackMap;


//-----------------------------------------------------------------------------------------------
void* operator new(size_t numBytes)
{
	size_t* ptr = (size_t*)malloc(numBytes + sizeof(size_t));
	//DebuggerPrintf("Alloc %p of %u bytes.\n", ptr, numBytes);
	++g_numAllocations;
	g_totalAllocatedBytes += numBytes;
	g_bytesAllocatedSinceLastUpdate += numBytes;
	if (g_totalAllocatedBytes > g_currentHighwaterBytes)
	{
		g_currentHighwaterBytes = g_totalAllocatedBytes;
	}
	*ptr = numBytes;
	ptr++;

	if (g_isCallstackSysInitialized)
	{
		callstack_t* cStack = CallstackFetch(0);
		cStack->bytes = numBytes;
		g_callStackMap.insert(ptr, cStack);
	}

	Profiler::AddAllocation(numBytes);

	return ptr;
}


//-----------------------------------------------------------------------------------------------
void operator delete(void* ptr)
{
	//size_t* myPtr = (size_t*)ptr;
	//myPtr--;
	//g_totalAllocatedBytes -= *myPtr;
	//g_bytesFreedSinceLastUpdate += *myPtr;
	//--g_numAllocations;
	//if (g_isCallstackSysInitialized)
	//{
	//	g_callStackMap.erase(ptr);
	//}
	//Profiler::SubtractAllocation(*myPtr);
	//free(myPtr);
}


//-----------------------------------------------------------------------------------------------
void MemoryAnalyticsStartup()
{
	g_numStartupAllocations = g_numAllocations;
	g_totalAllocatedBytesStartup = g_totalAllocatedBytes;
	Logger::StartLogging("NetSession");
	Logger::Printf("Number of allocations at startup: %u\n", g_numStartupAllocations);
	Logger::Printf("Number of allocated bytes at startup: %u\n", g_totalAllocatedBytesStartup);
	DebuggerPrintf("Number of allocations at startup: %u\nNumber of allocated bytes: %u\n", g_numStartupAllocations, g_totalAllocatedBytesStartup);
}


//-----------------------------------------------------------------------------------------------
callstack_line_t* GetOriginatingCall(callstack_t* stack, callstack_line_t* linePtr)
{
	for (size_t i = 1; i < stack->frame_count; i++)
	{
		if (strstr(linePtr[i].filename, "\\code\\"))
		{
			return &linePtr[i];
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
void FlushCallstacks()
{
	for (int i = 0; i < g_callStackMap.get_count(); i++)
	{
		callstack_t* cStack = g_callStackMap[i];
		DebuggerPrintf("New callstack!  Bytes leaked: %u\n", cStack->bytes);
		callstack_line_t* linePtr = CallstackGetLines(cStack);
		callstack_line_t* fileLine = GetOriginatingCall(cStack, linePtr);
		DebuggerPrintf("Originating call: \n%s(%i)\n", fileLine->filename, fileLine->line);
		DebuggerPrintf("Callstack:\n");
		for (size_t j = 0; j < cStack->frame_count; j++)
		{
			DebuggerPrintf("%s(%i)\n", linePtr[j].filename, linePtr[j].line);
		}
	}
}


//-----------------------------------------------------------------------------------------------
void MemoryAnalyticsShutdown()
{
	Logger::Printf("Number of allocations at shutdown: %u\n", g_numAllocations);
	Logger::Printf("Number of allocated bytes at shutdown: %u\n", g_totalAllocatedBytes);
	Logger::Printf("Number of leaked allocations: %i\n", g_numAllocations - g_numStartupAllocations);
	Logger::Printf("Number of leaked bytes: %i\n", g_totalAllocatedBytes - g_totalAllocatedBytesStartup);
	Logger::StopLogging();
	DebuggerPrintf("Number of allocations at shutdown: %u\nNumber of allocated bytes: %u\n", g_numAllocations, g_totalAllocatedBytes);
	if (!g_callStackMap.empty())
	{
		size_t numLeaks = g_callStackMap.get_count();
		FlushCallstacks();

		//Using remaining callstacks as indicator for leaks, as I sometimes have fewer allocations on shutdown than startup
		ERROR_AND_DIE(Stringf("%u leaks detected!", numLeaks));
	}
}

bool g_isDebuggingMemory = false;
//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(MemoryDebug, args)
{
	UNUSED(args);
#ifdef MEMORY_DETECTION_MODE
	g_isDebuggingMemory = !g_isDebuggingMemory;
#endif
}


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(MemoryFlush, args)
{
	UNUSED(args);
#if defined(MEMORY_DETECTION_MODE) && MEMORY_DETECTION_MODE == MEMORY_DETECTION_VERBOSE
	FlushCallstacks();
#endif
}
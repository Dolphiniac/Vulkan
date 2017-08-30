#pragma once
#if !defined( __DEBUG_CALLSTACK__ )
#define __DEBUG_CALLSTACK__
#include <cstdlib>

/************************************************************************/
/*                                                                      */
/* INCLUDE                                                              */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* DEFINES AND CONSTANTS                                                */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* MACROS                                                               */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* TYPES                                                                */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* STRUCTS                                                              */
/*                                                                      */
/************************************************************************/
struct callstack_t
{
	void** frames;
	size_t frame_count;
	size_t bytes;
};

struct callstack_line_t 
{
	char filename[256];
	char function_name[256];
	size_t line;
	size_t offset;
};

/************************************************************************/
/*                                                                      */
/* CLASSES                                                              */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* GLOBAL VARIABLES                                                     */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* FUNCTION PROTOTYPES                                                  */
/*                                                                      */
/************************************************************************/

bool CallstackSystemInit();
void CallstackSystemDeinit();
callstack_t* CallstackFetch( size_t skip_frames );
extern bool g_isCallstackSysInitialized;


// Single Threaded - only from debug output thread (if I need the string names elsewhere
// then I need to make a "debug" job consumer)
callstack_line_t* CallstackGetLines( callstack_t *cs );


class CSystemAllocator
{
public:
	void init(void* ptr) 
	{
		if (!ptr)
		{
			startPtr = malloc(1024 * 1024 * 1024);
			currPtr = startPtr;
		}
	}
	void* alloc(size_t numBytes)
	{
		void* toReturn = currPtr;
		unsigned char* workingPtr = (unsigned char*)currPtr;
		workingPtr += numBytes;
		currPtr = workingPtr;
		return toReturn;
	}
	void deinit()
	{
		free(startPtr);
	}

	void* currPtr;
	void* startPtr;
};


class CBuffer
{
public:
	void init(void* startData, size_t numBytes)
	{
		UNUSED(numBytes);
		startPtr = startData;
		currPtr = startPtr;
		//currPtr = (unsigned char*)startPtr + numBytes;
	}
	template<typename T>
	T* alloc()
	{
		T* result = (T*)currPtr;
		unsigned char* workingPtr = (unsigned char*)currPtr;
		workingPtr += sizeof(T);
		currPtr = workingPtr;
		return result;
	}
	void* get_front() { return currPtr; }
	void* startPtr;
	void* currPtr;
};

#endif 

/************************************************************************/
/*                                                                      */
/* INCLUDE                                                              */
/*                                                                      */
/************************************************************************/
#include "callstack.h"
#include "Engine/Core/ErrorWarningAssert.hpp"

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <Windows.h>
#include <DbgHelp.h>

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
#define MAX_SYMBOL_NAME_LENGTH 128
#define MAX_FILENAME_LENGTH 1024
#define MAX_DEPTH 128

/************************************************************************/
/*                                                                      */
/* TYPES                                                                */
/*                                                                      */
/************************************************************************/

// SymInitialize()
typedef BOOL (__stdcall *sym_initialize_t)( IN HANDLE hProcess, IN PSTR UserSearchPath, IN BOOL fInvadeProcess );
typedef BOOL (__stdcall *sym_cleanup_t)( IN HANDLE hProcess );
typedef BOOL (__stdcall *sym_from_addr_t)( IN HANDLE hProcess, IN DWORD64 Address, OUT PDWORD64 Displacement, OUT PSYMBOL_INFO Symbol );

typedef BOOL (__stdcall *sym_get_line_t)( IN HANDLE hProcess, IN DWORD64 dwAddr, OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE64 Symbol );

/************************************************************************/
/*                                                                      */
/* STRUCTS                                                              */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* CLASSES                                                              */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* LOCAL VARIABLES                                                      */
/*                                                                      */
/************************************************************************/
static HMODULE gDebugHelp;
static HANDLE gProcess;
static SYMBOL_INFO  *gSymbol;
static CSystemAllocator gStackPool;

// only called from single thread - so can use a shared buffer
static char gFileName[MAX_FILENAME_LENGTH];
static callstack_line_t gCallstackBuffer[MAX_DEPTH];
bool g_isCallstackSysInitialized = false;

static sym_initialize_t LSymInitialize;
static sym_cleanup_t LSymCleanup;
static sym_from_addr_t LSymFromAddr;
static sym_get_line_t LSymGetLineFromAddr64;


/************************************************************************/
/*                                                                      */
/* GLOBAL VARIABLES                                                     */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* LOCAL FUNCTIONS                                                      */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* EXTERNAL FUNCTIONS                                                   */
/*                                                                      */
/************************************************************************/

	
//------------------------------------------------------------------------
bool CallstackSystemInit()
{
#ifndef __USING_UWP
	gStackPool.init(nullptr);

	gDebugHelp = LoadLibraryA( "dbghelp.dll" );
	ASSERT_OR_DIE( gDebugHelp != nullptr, "Could not initialize dbghelp.dll" );
	LSymInitialize = (sym_initialize_t)GetProcAddress( gDebugHelp, "SymInitialize" );
	LSymCleanup = (sym_cleanup_t)GetProcAddress( gDebugHelp, "SymCleanup" );
	LSymFromAddr = (sym_from_addr_t)GetProcAddress( gDebugHelp, "SymFromAddr" );
	LSymGetLineFromAddr64 = (sym_get_line_t)GetProcAddress( gDebugHelp, "SymGetLineFromAddr64" );

	gProcess = GetCurrentProcess();
	LSymInitialize( gProcess, NULL, TRUE );

	gSymbol = (SYMBOL_INFO *)malloc(sizeof(SYMBOL_INFO) + (MAX_FILENAME_LENGTH * sizeof(char)));
	gSymbol->MaxNameLen   = MAX_FILENAME_LENGTH;
	gSymbol->SizeOfStruct = sizeof( SYMBOL_INFO );

	g_isCallstackSysInitialized = true;


   return true;
#else
	return false;
#endif
}

//------------------------------------------------------------------------
static int gCallstackCount = 0;
void CallstackSystemDeinit()
{
	LSymCleanup( gProcess );
#ifndef __USING_UWP
	FreeLibrary(gDebugHelp);
#endif
	gDebugHelp = NULL;

	if (gCallstackCount != 0) 
	{
		gCallstackCount = 0;
	}

	g_isCallstackSysInitialized = false;

	gStackPool.deinit();
}

//------------------------------------------------------------------------
// static void OnFreeCallstack( IAllocator*, void *ptr ) 
// {
// 	AtomicDecrement( &gCallstackCount );
// }

//------------------------------------------------------------------------
callstack_t* CallstackFetch( size_t skip_frames )
{
	void *stack[MAX_DEPTH];
	uint32_t frames = CaptureStackBackTrace( 1 + skip_frames, MAX_DEPTH, stack, NULL );
		
	//AtomicIncrement( &gCallstackCount );

	size_t size = sizeof(callstack_t) + sizeof(void*) * frames;
	void *buf_data = gStackPool.alloc(size);// (&gStackPool, size, OnFreeCallstack);
	CBuffer buf;
	buf.init( buf_data, size );
	
	callstack_t *cs = buf.alloc<callstack_t>();
	cs->frames = (void**) buf.get_front();
	//callstack_t* cs = (callstack_t*)gStackPool.alloc(size);
	//cs->frames = (void**)cs;
	cs->frame_count = frames;
	memcpy( cs->frames, stack, sizeof(void*) * frames );

	return cs;
}

//------------------------------------------------------------------------
// Should only be called from the debug trace thread.  
callstack_line_t* CallstackGetLines( callstack_t *cs ) 
{
	IMAGEHLP_LINE64 LineInfo; 
	DWORD LineDisplacement = 0; // Displacement from the beginning of the line 
	LineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	size_t count = cs->frame_count;
	for (size_t i = 0; i < count; ++i) 
	{
		callstack_line_t *line = &(gCallstackBuffer[i]);
		DWORD64 ptr = (DWORD64)(cs->frames[i]);
		LSymFromAddr( gProcess, ptr, 0, gSymbol );
		strncpy_s(line->function_name, gSymbol->Name, _TRUNCATE);
		//StringCopy( line->function_name, 128, gSymbol->Name  );

		BOOL bRet = LSymGetLineFromAddr64( 
			GetCurrentProcess(), // Process handle of the current process 
			ptr, // Address 
			&LineDisplacement, // Displacement will be stored here by the function 
			&LineInfo );         // File name / line information will be stored here 

		if (bRet) 
		{
			line->line = LineInfo.LineNumber;

			char const *filename = /*StringFindLast*/strstr( LineInfo.FileName, "\\Code" );
			if (filename == NULL) 
			{
				filename = LineInfo.FileName;
			} 
			else 
			{
				filename += 6; // skip to the important bit - so that it can be double clicked in Output
			}

			strncpy_s(line->filename, filename, _TRUNCATE);
			//StringCopy( line->filename, 128, filename );
			line->offset = LineDisplacement;
		}
		else 
		{
			line->line = 0;
			line->offset = 0;
			strcpy_s(line->filename, "N/A");
			//StringCopy( line->filename, 128, "N/A" );
		}
	}

	return gCallstackBuffer;
}


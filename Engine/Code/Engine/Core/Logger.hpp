#pragma once

#include "Engine/Memory/ThreadSafeSTL.hpp"

#include <string>
#include <thread>
#include <tuple>
#include <atomic>


//-----------------------------------------------------------------------------------------------
enum EWarningLevel
{
	WARNING_ERROR,
	WARNING_MAJOR,
	WARNING_MINOR,
	WARNING_NEGLIGIBLE
};


//-----------------------------------------------------------------------------------------------
class Logger
{
public:
	static void Printf(const char* format, ...);
	static void Printf(EWarningLevel warningLevel, const char* format, ...);
	static void TagPrintf(const char* tag, const char* format, ...);
	static void TagPrintf(EWarningLevel warningLevel, const char* tag, const char* format, ...);
	static void StartLogging(const std::string& logPrefix);
	static void StopLogging();
	static void Flush();

private:
	static void SlavePumpLogging();
	static void Printvf(const char* format, va_list& vargs);
	static ThreadSafeQueue<std::string> s_messageQueue;
	static FILE* s_file;
	static std::string s_logName;
	static std::thread s_slaveLogger;
	static bool s_isRunning;
	static CriticalSection s_flushRequestCS;
	static bool s_needsFlush;
	static bool s_hasFlushed;
};
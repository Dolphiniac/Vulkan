#include "Engine/Core/Logger.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <stdarg.h>
#include <time.h>
#include <thread>


//-----------------------------------------------------------------------------------------------
ThreadSafeQueue<std::string> Logger::s_messageQueue;
FILE* Logger::s_file;
std::string Logger::s_logName;
std::thread Logger::s_slaveLogger;
bool Logger::s_isRunning = false;
CriticalSection Logger::s_flushRequestCS;
bool Logger::s_needsFlush = false;
bool Logger::s_hasFlushed = false;


//-----------------------------------------------------------------------------------------------
void Logger::Printvf(const char* format, va_list& vargs)
{
	const int MESSAGE_MAX_LENGTH = 1024;
	char message[MESSAGE_MAX_LENGTH];
	vsnprintf_s(message, MESSAGE_MAX_LENGTH, format, vargs);
	message[MESSAGE_MAX_LENGTH - 1] = '\0';
	s_messageQueue.Enqueue(message);
}


//-----------------------------------------------------------------------------------------------
void Logger::Printf(const char* format, ...)
{
	va_list vargs;
	va_start(vargs, format);
	Printvf(format, vargs);
	va_end(vargs);
}


//-----------------------------------------------------------------------------------------------
void Logger::Printf(EWarningLevel warningLevel, const char* format, ...)
{
	std::string newFormat = Stringf("[W%i] %s", warningLevel, format);
	va_list vargs;
	va_start(vargs, format);
	Printvf(newFormat.c_str(), vargs);
	va_end(vargs);
}


//-----------------------------------------------------------------------------------------------
void Logger::TagPrintf(const char* tag, const char* format, ...)
{
	std::string newFormat = Stringf("%s: %s", tag, format);
	va_list vargs;
	va_start(vargs, format);
	Printvf(newFormat.c_str(), vargs);
	va_end(vargs);
}


//-----------------------------------------------------------------------------------------------
void Logger::TagPrintf(EWarningLevel warningLevel, const char* tag, const char* format, ...)
{
	std::string newFormat = Stringf("[W%i] %s: %s", warningLevel, tag, format);
	va_list vargs;
	va_start(vargs, format);
	Printvf(newFormat.c_str(), vargs);
	va_end(vargs);
}


//-----------------------------------------------------------------------------------------------
void Logger::Flush()
{
	for (;;)
	{
		//Don't want to pump too often
		std::this_thread::yield();
		//Scope lock so I don't repeat myself
		CriticalSectionGuard csg(&s_flushRequestCS);
		if (!s_hasFlushed && !s_needsFlush)
		{
			s_needsFlush = true;
			break;
		}
	}

	for (;;)
	{
		std::this_thread::yield();
		CriticalSectionGuard csg(&s_flushRequestCS);
		if (s_hasFlushed)
		{
			s_hasFlushed = false;
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
static std::string GetLogName(const std::string& logPrefix)
{
	time_t currTime = time(NULL);
	tm currTimeStruct;
	localtime_s(&currTimeStruct, &currTime);

	std::string logName = "Data/Logs/" + logPrefix + "_";
	logName += std::to_string(currTimeStruct.tm_year + 1900);
	if (currTimeStruct.tm_mon < 10)
	{
		logName.push_back('0');
	}
	logName += std::to_string(currTimeStruct.tm_mon);
	if (currTimeStruct.tm_mday < 10)
	{
		logName.push_back('0');
	}
	logName += std::to_string(currTimeStruct.tm_mday);
	logName += "_";
	if (currTimeStruct.tm_hour < 10)
	{
		logName.push_back('0');
	}
	logName += std::to_string(currTimeStruct.tm_hour);
	if (currTimeStruct.tm_min < 10)
	{
		logName.push_back('0');
	}
	logName += std::to_string(currTimeStruct.tm_min);
	if (currTimeStruct.tm_sec < 10)
	{
		logName.push_back('0');
	}
	logName += std::to_string(currTimeStruct.tm_sec);
	logName += ".log";

	return logName;
}

//-----------------------------------------------------------------------------------------------
void Logger::StartLogging(const std::string& logPrefix)
{
	s_logName = logPrefix;

	std::string logName = GetLogName(logPrefix);


	fopen_s(&s_file, logName.c_str(), "w+");
	s_isRunning = true;
	s_slaveLogger = std::thread(SlavePumpLogging);
}


//-----------------------------------------------------------------------------------------------
void Logger::StopLogging()
{
	s_isRunning = false;

	s_slaveLogger.join();

	fflush(s_file);
	unsigned int length = ftell(s_file);
	rewind(s_file);
	FILE* copyFile;
	fopen_s(&copyFile, ("Data/Logs/" + s_logName + ".log").c_str(), "w");
	void* buffer = new unsigned char[length];
	fread(buffer, 1, length, s_file);
	//Why -6?  I don't know.  But it was copying 6 grave accent Is to the copy file
	fwrite(buffer, 1, length - 6, copyFile);
	fclose(s_file);
	fclose(copyFile);
	delete[] buffer;
	s_messageQueue.clear();
}


//-----------------------------------------------------------------------------------------------
void Logger::SlavePumpLogging()
{
	std::string currMessage;
	while (s_isRunning)
	{
		while (s_messageQueue.Dequeue(&currMessage))
		{
			//First, pump existing messages
			fprintf(s_file, currMessage.c_str());
		}
		s_flushRequestCS.Enter();
		if (s_needsFlush)
		{
			s_flushRequestCS.Leave();
			while (s_messageQueue.Dequeue(&currMessage))
			{
				//Pump any messages in queue before flushing
				fprintf(s_file, currMessage.c_str());
			}
			fflush(s_file);
			s_flushRequestCS.Enter();
			s_needsFlush = false;
			s_hasFlushed = true;
		}
		s_flushRequestCS.Leave();

		std::this_thread::yield();
	}

	while (s_messageQueue.Dequeue(&currMessage))
	{
		//One last flush of message queue
		fprintf(s_file, currMessage.c_str());
	}
}
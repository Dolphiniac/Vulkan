#pragma once


//-----------------------------------------------------------------------------------------------
namespace BytePacker
{
	void WriteForward(const void* toWrite, void** destPtr, int dataSize);
	void WriteBackward(const void* toWrite, void** destPtr, int dataSize);
	void ReadForward(void* outData, void** destPtr, int dataSize);
	void ReadBackward(void* outData, void** destPtr, int dataSize);
};
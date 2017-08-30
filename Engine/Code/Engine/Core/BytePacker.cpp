#include "Engine/Core/BytePacker.hpp"

#include <memory.h>


//-----------------------------------------------------------------------------------------------
void BytePacker::WriteForward(const void* toWrite, void** destPtr, int dataSize)
{
	memcpy(*destPtr, toWrite, dataSize);
	byte* advancePtr = (byte*)*destPtr;
	advancePtr += dataSize;
	*destPtr = advancePtr;
}


//-----------------------------------------------------------------------------------------------
void BytePacker::WriteBackward(const void* toWrite, void** destPtr, int dataSize)
{
	byte* workingPtr = (byte*)(*destPtr);
	for (int i = dataSize - 1; i >= 0; i--, workingPtr++)
	{
		byte* dst = workingPtr;
		const byte* src = (const byte*)toWrite + i;

		*dst = *src;
	}

	*destPtr = workingPtr;
}


//-----------------------------------------------------------------------------------------------
void BytePacker::ReadForward(void* outData, void** destPtr, int dataSize)
{
	memcpy(outData, *destPtr, dataSize);

	byte* advancePtr = (byte*)*destPtr;
	advancePtr += dataSize;

	*destPtr = advancePtr;
}


//-----------------------------------------------------------------------------------------------
void BytePacker::ReadBackward(void* outData, void** destPtr, int dataSize)
{
	for (int i = dataSize - 1; i >= 0; i--)
	{
		byte* dst = (byte*)outData + i;
		const byte* src = (const byte*)*destPtr + (dataSize - 1 - i);

		*dst = *src;
	}

	byte* advancePtr = (byte*)*destPtr;
	advancePtr += dataSize;

	*destPtr = advancePtr;
}
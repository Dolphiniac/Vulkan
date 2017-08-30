#pragma once

#include "Engine/Core/ErrorWarningAssert.hpp"
#include <stdlib.h>


//-----------------------------------------------------------------------------------------------
class LinearAllocator
{
public:
	LinearAllocator(size_t bytes)
		:m_maxAllocatedBytes(bytes)
	{
		m_startPtr = malloc(bytes);
		m_currPtr = m_startPtr;
	}

	template<typename T>
	T* Alloc()
	{
		T* result = (T*)m_currPtr;
		char* bytePtr = (char*)m_currPtr;
		bytePtr += sizeof(T);
		ASSERT_OR_DIE(bytePtr - m_startPtr < m_maxAllocatedBytes, "Ran out of space in linear allocator!");
		m_currPtr = bytePtr;

		return result;
	}

	void* Get(size_t offset)
	{
		char* result = (char*)m_startPtr;
		return result + offset;
	}

	~LinearAllocator()
	{
		free(m_startPtr);
	}

private:
	void* m_startPtr;
	void* m_currPtr;
	size_t m_maxAllocatedBytes;
};
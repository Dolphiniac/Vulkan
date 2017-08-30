//-----------------------------------------------------------------------------------------------
//INLINE FILE FOR OBJECT POOL TEMPLATE CLASS
//-----------------------------------------------------------------------------------------------

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <stdlib.h>


//-----------------------------------------------------------------------------------------------
template<typename T>
ObjectPool<T>::ObjectPool(size_t maxNumObjects)
	: m_freeListHead(nullptr)
{
	ASSERT_OR_DIE(maxNumObjects > 0, "Cannot allocate fewer than 1 object");
	size_t sizeOfPage = Max(sizeof(T), sizeof(Page));
	size_t numBytes = maxNumObjects * sizeOfPage;

	m_initialPointer = malloc(numBytes);

	for (size_t i = 0; i < maxNumObjects; i++)
	{
		Page* currPage = (Page*)((size_t)m_initialPointer + i * sizeOfPage);
		currPage->nextPage = m_freeListHead;
		m_freeListHead = currPage;
	}
}


//-----------------------------------------------------------------------------------------------
template<typename T>
ObjectPool<T>::~ObjectPool()
{
	free(m_initialPointer);
}


//-----------------------------------------------------------------------------------------------
template<typename T>
template<typename... Args>
T* ObjectPool<T>::Create(Args... ctorArgs)
{
	ASSERT_OR_DIE(m_freeListHead, "Object pool ran out of space!");

	T* currObjectMemory = (T*)m_freeListHead;
	m_freeListHead = m_freeListHead->nextPage;
	new (currObjectMemory) T(ctorArgs...);
	
	return currObjectMemory;
}


//-----------------------------------------------------------------------------------------------
template<typename T>
void ObjectPool<T>::Free(T* toFree)
{
	Page* currPage = (Page*)toFree;
	currPage->nextPage = m_freeListHead;
	m_freeListHead = currPage;
}
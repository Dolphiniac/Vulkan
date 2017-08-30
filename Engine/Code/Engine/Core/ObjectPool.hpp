#pragma once


//-----------------------------------------------------------------------------------------------
template<typename T>
class ObjectPool
{
public:
	ObjectPool(size_t maxNumObjects);
	~ObjectPool();
	template<typename... Args> T* Create(Args... args);
	void Free(T* toFree);

private:
	struct Page
	{
		Page* nextPage;
	};

private:
	Page* m_freeListHead;
	void* m_initialPointer;

};

#include "Engine/Core/ObjectPool.inl"
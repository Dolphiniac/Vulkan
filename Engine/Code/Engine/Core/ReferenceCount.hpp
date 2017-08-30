#pragma once

#include <stdlib.h>


//-----------------------------------------------------------------------------------------------
namespace RefCount
{
	template<typename T, typename... Args>
	T* CreateAndAcquire(Args... args)
	{
		size_t* block = (size_t*)malloc(sizeof(T) + sizeof(size_t));
		*block = 1;
		++block;
		T* result = (T*)block;
		new (result) T(args...);

		return result;
	}

	void Acquire(void* refCountedPtr)
	{
		size_t* memory = (size_t*)refCountedPtr;
		--memory;
		++(*memory);
	}

	template<typename T>
	void Release(T* refCountedPtr)
	{
		size_t* memory = (size_t*)refCountedPtr;
		--memory;
		--(*memory);
		if (*memory == 0)
		{
			refCountedPtr->~T();
			free(memory);
		}
	}

	template<typename T>
	void Release(T& refCountedObject)
	{
		Release<T>(&refCountedObject);
	}

	template<typename T>
	class ScopedAcquirer
	{
		ScopedAcquirer(T* refCountedPtr)
			: m_refCountedPtr(refCountedPtr)
		{
			Acquire(refCountedPtr);
		}
		~ScopedAcquirer()
		{
			Release<T>(m_refCountedPtr);
		}
	private:
		T* m_refCountedPtr;
	};
}

#define REF_ACQUIRE_SCOPE(Typename, Ptr) RefCount::ScopedAcquirer<Typename> raiiRefCountAcquirer(Ptr);
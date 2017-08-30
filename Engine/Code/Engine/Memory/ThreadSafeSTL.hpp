#pragma once

#include "Engine/Memory/CriticalSection.hpp"

#include <queue>


//-----------------------------------------------------------------------------------------------
template<typename T>
class ThreadSafeQueue : protected std::deque<T>
{
public:
	void Enqueue(const T& datum)
	{
		m_cs.Enter();
		push_back(datum);
		m_cs.Leave();
	}
	bool Dequeue(T* outValue)
	{
		CriticalSectionGuard csg(&m_cs);
		if (empty())
		{
			return false;
		}
		*outValue = front();
		pop_front();
		return true;
	}
	void clear()
	{
		std::deque<T>::clear();
	}

private:
	CriticalSection m_cs;
};
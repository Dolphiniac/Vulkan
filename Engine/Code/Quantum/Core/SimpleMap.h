#pragma once

#include "Engine/Core/ErrorWarningAssert.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
template<typename T>
class QuSimpleMap
{
public:
	struct Pair
	{
		uint32 key;
		T value;
	};

public:
	void Insert(uint32 key, const T& element)
	{
		Pair p;
		p.key = key;
		p.value = element;

		m_list.push_back(std::move(p));
	}
	void Remove(uint32 key)
	{
		for (auto iter = m_list.begin(); iter != m_list.end(); iter++)
		{
			Pair& p = *iter;
			if (p.key == key)
			{
				m_list.erase(iter);
				return;
			}
		}
	}
	const T* Find(uint32 key) const
	{
		uint32 count = m_list.size();
		FOR_COUNT(index, count)
		{
			const Pair& p = m_list[index];
			if (p.key == key)
			{
				return &p.value;
			}
		}

		return nullptr;
	}
	uint32 GetSize() const { return m_list.size(); }
	uint32 GetKeyAtIndex(uint32 index) { ASSERT_OR_DIE(m_list.size() > index, "Out of range index\n"); return m_list[index].key; }
	T* GetValueAtIndex(uint32 index) { ASSERT_OR_DIE(m_list.size() > index, "Out of range index\n"); return &m_list[index].value; }
	Pair& GetPairAtIndex(uint32 index) { ASSERT_OR_DIE(m_list.size() > index, "Out of range index\n"); return m_list[index]; }

private:
	std::vector<Pair> m_list;
};
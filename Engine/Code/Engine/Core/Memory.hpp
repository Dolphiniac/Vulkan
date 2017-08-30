#pragma once

#include <cstdlib>

#define MEMORY_TRACKING_BASIC 0
#define MEMORY_TRACKING_VERBOSE 1

#define MEMORY_TRACKING MEMORY_TRACKING_BASIC


//-----------------------------------------------------------------------------------------------
extern unsigned int g_numAllocations;
extern unsigned int g_totalAllocatedBytes;
extern unsigned int g_numStartupAllocations;
extern unsigned int g_totalAllocatedBytesStartup;
extern unsigned int g_currentHighwaterBytes;
extern unsigned int g_bytesAllocatedSinceLastUpdate;
extern unsigned int g_bytesFreedSinceLastUpdate;


//-----------------------------------------------------------------------------------------------
void MemoryAnalyticsStartup();
void MemoryAnalyticsShutdown();


//-----------------------------------------------------------------------------------------------
template<typename T, typename U>
struct MapNode
{
	T first;
	U second;
	MapNode<T, U>* next;
};


//-----------------------------------------------------------------------------------------------
template<typename T, typename U>
class UntrackedMap
{
public:
	UntrackedMap() : head(nullptr) {}
	void insert(const T& first, const U& second)
	{
		if (!head)
		{
			head = (MapNode<T, U>*)malloc(sizeof(MapNode<T, U>));
			memcpy(&head->first, &first, sizeof(T));
			//head->first = first;
			memcpy(&head->second, &second, sizeof(U));
			//head->second = second;
			head->next = nullptr;
			return;
		}

		MapNode<T, U>* curr = head;
		while (curr->next)
		{
			if (curr->first == first)
			{
				curr->second = second;
				return;
			}
			curr = curr->next;
		}

		curr->next = (MapNode<T, U>*)malloc(sizeof(MapNode<T, U>));
		curr = curr->next;
		memcpy(&curr->first, &first, sizeof(T));
		//curr->first = first;
		memcpy(&curr->second, &second, sizeof(U));
		//curr->second = second;
		curr->next = nullptr;
	}
	void erase(const T& key)
	{
		if (!head)
		{
			return;
		}
		if (head->first == key)
		{
			MapNode<T, U>* temp = head->next;
			free(head);
			head = temp;
			return;
		}

		MapNode<T, U>* curr = head;

		while (curr->next)
		{
			if (curr->next->first == key)
			{
				MapNode<T, U>* toFree = curr->next;
				curr->next = toFree->next;
				free(toFree);
				return;
			}
			curr = curr->next;
		}
	}
	int get_count()
	{
		MapNode<T, U>* curr = head;
		int result = 0;
		while (curr)
		{
			curr = curr->next;
			result++;
		}

		return result;
	}
	U& operator[](int index)
	{
		if (index >= get_count())
		{
			throw std::out_of_range(nullptr);
		}
		MapNode<T, U>* curr = head;
		for (int i = 0; i < index; i++)
		{
			curr = curr->next;
		}

		return curr->second;
	}
	U& at(const T& key)
	{
		MapNode<T, U>* curr = head;
		while (curr)
		{
			if (curr->first == key)
			{
				return curr->second;
			}

			curr = curr->next;
		}

		throw std::exception("Key does not exist");
	}
	bool empty() { return get_count() == 0; }

public:
	MapNode<T, U>* head;
};
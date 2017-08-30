#include "Engine/Core/EventSystem.hpp"


//-----------------------------------------------------------------------------------------------
EventSystem* g_eventSystem = nullptr;


//-----------------------------------------------------------------------------------------------
EventSystem::~EventSystem()
{
	for (EventPair& p : m_events)
	{
		delete p.second;
	}
}


//-----------------------------------------------------------------------------------------------
void EventSystem::TriggerEvent(const std::string& eventName, Event* eventData)
{
	EventSubscribers* subscribers = FindEvent(eventName);

	if (subscribers)
	{
		size_t numElements = subscribers->size();
		for (size_t i = 0; i < numElements; i++)
		{
			EventSubscriber& sub = subscribers->at(i);
			sub.eventFunc(eventData, sub.subscriber);
			while (numElements > subscribers->size())
			{
				numElements--;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void EventSystem::UnregisterFromEvent(void* subscriber, const std::string& eventName)
{
	EventSubscribers* subscribers = FindEvent(eventName);

	if (subscribers)
	{
		std::vector<EventSubscriber>::iterator iter = subscribers->begin();
		while (iter != subscribers->end())
		{
			if (iter->subscriber == subscriber)
			{
				iter = subscribers->erase(iter);
				break;
			}
			else
			{
				iter++;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void EventSystem::UnregisterFromAllEvents(void* subscriber)
{
	for (EventPair& p : m_events)
	{
		EventSubscribers* subscribers = p.second;

		if (subscribers)
		{
			std::vector<EventSubscriber>::iterator iter = subscribers->begin();
			while (iter != subscribers->end())
			{
				if (iter->subscriber == subscriber)
				{
					iter = subscribers->erase(iter);
					break;
				}
				else
				{
					iter++;
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void EventSystem::RegisterEvent(const std::string& eventName, EventCallback* eventFunc, void* subscriber)
{
	EventSubscribers* subscribers = FindOrCreateEvent(eventName);

	EventSubscriber newSub = { eventFunc, subscriber };
	subscribers->push_back(newSub);
}


//-----------------------------------------------------------------------------------------------
EventSubscribers* EventSystem::FindOrCreateEvent(const std::string& eventName)
{
	EventSubscribers* result = FindEvent(eventName);

	if (result)
	{
		return result;
	}

	size_t newHash = std::hash<std::string>()(eventName);

	EventSubscribers* subs = new EventSubscribers();
	m_events.insert(std::make_pair(newHash, subs));

	return subs;
}


//-----------------------------------------------------------------------------------------------
EventSubscribers* EventSystem::FindEvent(const std::string& eventName)
{
	size_t eventHash = std::hash<std::string>()(eventName);

	EventMapIter iter = m_events.find(eventHash);

	if (iter == m_events.end())
	{
		return nullptr;
	}

	return iter->second;
}
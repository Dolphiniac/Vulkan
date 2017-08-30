#pragma once

//-----------------------------------------------------------------------------------------------
// LOVINGLY TRANSCRIBED FROM NICK'S EVENT SYSTEM
//-----------------------------------------------------------------------------------------------

#include <string>
#include <map>
#include <vector>


//-----------------------------------------------------------------------------------------------
extern class EventSystem* g_eventSystem;


//-----------------------------------------------------------------------------------------------
// Should always be inherited
struct Event {};


//-----------------------------------------------------------------------------------------------
// COMMON EVENT TYPES
//-----------------------------------------------------------------------------------------------
struct TickEvent : public Event
{
	float deltaSeconds;
};


//-----------------------------------------------------------------------------------------------
// END COMMON EVENT TYPES
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
typedef void EventCallback(Event* eventData, void* subArg);


//-----------------------------------------------------------------------------------------------
struct EventSubscriber
{
	EventCallback* eventFunc;
	void* subscriber;
};
typedef std::vector<EventSubscriber> EventSubscribers;


//-----------------------------------------------------------------------------------------------
typedef std::pair<const size_t, EventSubscribers*> EventPair;
typedef std::map<size_t, EventSubscribers*> EventMap;
typedef EventMap::iterator EventMapIter;


//-----------------------------------------------------------------------------------------------
template <class Subscriber, void(Subscriber::*Func)(Event*)>
void MethodEventStub(Event* eventData, void* subArg)
{
	Subscriber* sub = (Subscriber*)subArg;
	(sub->*Func)(eventData);
}


//-----------------------------------------------------------------------------------------------
class EventSystem
{
public:
	~EventSystem();

	template <class Subscriber, void(Subscriber::*Func)(Event*)>
	void RegisterEvent(const std::string& eventName, Subscriber* sub)
	{
		RegisterEvent(eventName, MethodEventStub<Subscriber, Func>, sub);
	}

	void TriggerEvent(const std::string& eventName, Event* eventData);

	void UnregisterFromEvent(void* subscriber, const std::string& eventName);
	void UnregisterFromAllEvents(void* subscriber);

private:
	void RegisterEvent(const std::string& eventName, EventCallback* eventFunc, void* subscriber);

	EventSubscribers* FindOrCreateEvent(const std::string& eventName);
	EventSubscribers* FindEvent(const std::string& eventName);

	EventMap m_events;
};

#include "Quantum/Core/EventSystem.h"


//-----------------------------------------------------------------------------------------------
STATIC QuEventSystem* QuEventSystem::s_instance = nullptr;


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
QuNamedProperties::~QuNamedProperties()
{
	for (std::pair<const QuHash, QuNamedPropertyBase*>& p : m_properties)
	{
		delete p.second;
	}
}


//-----------------------------------------------------------------------------------------------
QuEventSystem::~QuEventSystem()
{
	for (std::pair<const QuHash, std::vector<QuSubscriberBase*>>& p : m_objectSubscribers)
	{
		for (QuSubscriberBase* base : p.second)
		{
			delete base;
		}
	}
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
void QuNamedProperties::Remove(const QuString& name)
{
	auto propertyIter = m_properties.find(name);
	if (propertyIter != m_properties.end())
	{
		delete propertyIter->second;
		m_properties.erase(propertyIter);
	}
}


//-----------------------------------------------------------------------------------------------
STATIC QuEventSystem* QuEventSystem::GetInstance()
{
	if (!s_instance)
	{
		s_instance = new QuEventSystem();
	}

	return s_instance;
}


//-----------------------------------------------------------------------------------------------
STATIC void QuEventSystem::DestroySystem()
{
	SAFE_DELETE(s_instance);
}


//-----------------------------------------------------------------------------------------------
void QuEventSystem::Fire(const QuString& name, QuNamedProperties& params)
{
	auto funcIter = m_freeFuncSubscribers.find(name);
	if (funcIter != m_freeFuncSubscribers.end())
	{
		for (EventFunc FireEvent : funcIter->second)
		{
			FireEvent(params);
		}
	}

	auto methodIter = m_objectSubscribers.find(name);
	if (methodIter != m_objectSubscribers.end())
	{
		for (QuSubscriberBase* sub : methodIter->second)
		{
			sub->Execute(params);
		}
	}
}


//-----------------------------------------------------------------------------------------------
void QuEventSystem::Register(const QuString& name, EventFunc freeFunc)
{
	auto funcIter = m_freeFuncSubscribers.find(name);
	if (funcIter == m_freeFuncSubscribers.end())
	{
		m_freeFuncSubscribers.insert(std::make_pair(name, std::vector<EventFunc>()));
		funcIter = m_freeFuncSubscribers.find(name);
	}

	funcIter->second.push_back(freeFunc);
}


//-----------------------------------------------------------------------------------------------
void QuEventSystem::Register(const QuString& name, QuSubscriberBase* subscriber)
{
	auto methodIter = m_objectSubscribers.find(name);
	if (methodIter == m_objectSubscribers.end())
	{
		m_objectSubscribers.insert(std::make_pair(name, std::vector<QuSubscriberBase*>()));
		methodIter = m_objectSubscribers.find(name);
	}

	methodIter->second.push_back(subscriber);
}


//-----------------------------------------------------------------------------------------------
void QuEventSystem::UnregisterFunc(const QuString& name, EventFunc toUnregister)
{
	auto funcIter = m_freeFuncSubscribers.find(name);

	if (funcIter != m_freeFuncSubscribers.end())
	{
		for (auto iter = funcIter->second.begin(); iter != funcIter->second.end();)
		{
			EventFunc func = *iter;
			if (func == toUnregister)
			{
				iter = funcIter->second.erase(iter);
			}
			else
			{
				iter++;
			}
		}

		if (funcIter->second.empty())
		{
			m_freeFuncSubscribers.erase(funcIter);
		}
	}
}


//-----------------------------------------------------------------------------------------------
void QuEventSystem::UnregisterFromAll(EventFunc toUnregister)
{
	for (auto mapIter = m_freeFuncSubscribers.begin(); mapIter != m_freeFuncSubscribers.end();)
	{
		for (auto funcIter = mapIter->second.begin(); funcIter != mapIter->second.end();)
		{
			EventFunc func = *funcIter;
			if (func == toUnregister)
			{
				funcIter = mapIter->second.erase(funcIter);
			}
			else
			{
				funcIter++;
			}
		}
		if (mapIter->second.empty())
		{
			mapIter = m_freeFuncSubscribers.erase(mapIter);
		}
		else
		{
			mapIter++;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void QuEvent::Fire(const QuString& name)
{
	QuEventSystem* sys = QuEventSystem::GetInstance();

	QuNamedProperties params;
	sys->Fire(name, params);
}


//-----------------------------------------------------------------------------------------------
void QuEvent::Fire(const QuString& name, QuNamedProperties& params)
{
	QuEventSystem* sys = QuEventSystem::GetInstance();
	sys->Fire(name, params);
}


//-----------------------------------------------------------------------------------------------
void QuEvent::Register(const QuString& name, EventFunc freeFunc)
{
	QuEventSystem* sys = QuEventSystem::GetInstance();

	sys->Register(name, freeFunc);
}


//-----------------------------------------------------------------------------------------------
void QuEvent::UnregisterFunc(const QuString& name, EventFunc toUnregister)
{
	QuEventSystem* sys = QuEventSystem::GetInstance();

	sys->UnregisterFunc(name, toUnregister);
}


//-----------------------------------------------------------------------------------------------
void QuEvent::UnregisterFromAll(EventFunc toUnregister)
{
	QuEventSystem* sys = QuEventSystem::GetInstance();

	sys->UnregisterFromAll(toUnregister);
}
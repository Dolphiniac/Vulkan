//-----------------------------------------------------------------------------------------------
// INLINE FILE FOR EVENTSYSTEM.H
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
template<typename... T_Params>
QuNamedProperties::QuNamedProperties(T_Params... props)
{
	m_properties.clear();
	ConsumeParams(props...);
}


//-----------------------------------------------------------------------------------------------
template<typename T>
void QuNamedProperties::ConsumeParams(const QuString& paramName, const T& paramValue)
{
	Set(paramName, paramValue);
}


//-----------------------------------------------------------------------------------------------
template<typename T, typename... T_Params>
void QuNamedProperties::ConsumeParams(const QuString& paramName, const T& paramValue, T_Params... params)
{
	Set(paramName, paramValue);

	ConsumeParams(params...);
}


//-----------------------------------------------------------------------------------------------
template<typename T>
EPropertyGetResult QuNamedProperties::Get(const QuString& name, T& outDatum, EDataType* outDataType /* = nullptr */) const
{
	if (m_properties.empty())
	{
		return PGR_FAIL_NO_PROPERTIES;
	}
	auto propertyIter = m_properties.find(name);
	if (propertyIter == m_properties.end())
	{
		return PGR_FAIL_DOES_NOT_EXIST;
	}

	TNamedProperty<T>* prop = (TNamedProperty<T>*)propertyIter->second;
	if (outDataType)
	{
		*outDataType = prop->dataType;
	}
	if (typeid(prop->m_datum) != typeid(T))
	{
		return PGR_FAIL_TYPE_MISMATCH;
	}
	
	outDatum = prop->m_datum;

	return PGR_SUCCESS;
}


//-----------------------------------------------------------------------------------------------
template<typename T>
EPropertySetResult QuNamedProperties::Set(const QuString& name, const T& datum, EDataType dataType /* = DATATYPE_UNSPECIFIED */)
{
	auto propertyIter = m_properties.find(name);
	if (propertyIter == m_properties.end())
	{
		m_properties.insert(std::make_pair(name, new TNamedProperty<T>(datum)));
		return PSR_SUCCESS;
	}
	else
	{
		if (propertyIter->second->dataType != DATATYPE_UNSPECIFIED)
		{
			if (propertyIter->second->dataType != dataType)
			{
				return PSR_FAIL_TYPE_MISMATCH;
			}
		}
		delete propertyIter->second;

		propertyIter->second = new TNamedProperty<T>(datum);
		return PSR_SUCCESS;
	}
}


//-----------------------------------------------------------------------------------------------
// template<>
// void QuNamedProperties::Set(const QuString& name, const char* const& datum)
// {
// 	Set<QuString>(name, datum);
// }


//-----------------------------------------------------------------------------------------------
template<typename T>
void TSubscriber<T>::Execute(QuNamedProperties& params)
{
	(m_object->*m_method)(params);
}


//-----------------------------------------------------------------------------------------------
template<typename T>
void QuEventSystem::UnregisterObject(const QuString& name, const T* object)
{
	auto mapIter = m_objectSubscribers.find(name);

	if (mapIter != m_objectSubscribers.end())
	{
		for (auto objectIter = mapIter->second.begin(); objectIter != mapIter->second.end();)
		{
			QuSubscriberBase* base = *objectIter;
			T* obj = (T*)*base;	//Dereferencing will yield the object ptr in the subclass
			if (object == obj)
			{
				objectIter = mapIter->second.erase(objectIter);
			}
			else
			{
				objectIter++;
			}
		}

		if (mapIter->second.empty())
		{
			m_objectSubscribers.erase(mapIter);
		}
	}
}


//-----------------------------------------------------------------------------------------------
template<typename T>
void QuEventSystem::UnregisterFromAll(const T* object)
{
	for (auto mapIter = m_objectSubscribers.begin(); mapIter != m_objectSubscribers.end();)
	{
		for (auto objectIter = mapIter->second.begin(); objectIter != mapIter->second.end();)
		{
			QuSubscriberBase* base = *objectIter;
			void** vBase = (void**)base;
			T* obj = (T*)*vBase;	//Dereferencing will yield the object ptr in the subclass
			if (object == obj)
			{
				objectIter = mapIter->second.erase(objectIter);
			}
			else
			{
				objectIter++;
			}
		}

		if (mapIter->second.empty())
		{
			mapIter = m_objectSubscribers.erase(mapIter);
		}
		else
		{
			mapIter++;
		}
	}
}


//-----------------------------------------------------------------------------------------------
template<typename T, typename... T_Params>
void QuEvent::Fire(const QuString& name, const QuString& param1Name, const T& param1Value, T_Params... params)
{
	QuNamedProperties passParams;
	passParams.ConsumeParams(param1Name, param1Value, params...);

	QuEventSystem* sys = QuEventSystem::GetInstance();
	sys->Fire(name, passParams);
}


//-----------------------------------------------------------------------------------------------
template<typename T>
void QuEvent::Register(const QuString& name, T* obj, TMethodFunc<T> method)
{
	TSubscriber<T>* sub = new TSubscriber<T>(obj, method);

	QuEventSystem* sys = QuEventSystem::GetInstance();

	sys->Register(name, (QuSubscriberBase*)sub);
}


//-----------------------------------------------------------------------------------------------
template<typename T>
void QuEvent::UnregisterObject(const QuString& name, const T* object)
{
	QuEventSystem* sys = QuEventSystem::GetInstance();

	sys->UnregisterObject(name, object);
}


//-----------------------------------------------------------------------------------------------
template<typename T>
void QuEvent::UnregisterFromAll(const T* object)
{
	QuEventSystem* sys = QuEventSystem::GetInstance();

	sys->UnregisterFromAll(object);
}
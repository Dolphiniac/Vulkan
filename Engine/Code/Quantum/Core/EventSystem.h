#pragma once

#include "Quantum/Core/String.h"

#include <map>
#include <vector>


#pragma warning(disable: 4239)


//-----------------------------------------------------------------------------------------------
enum EPropertyGetResult
{
	PGR_SUCCESS,
	PGR_FAIL_NO_PROPERTIES,
	PGR_FAIL_DOES_NOT_EXIST,
	PGR_FAIL_TYPE_MISMATCH
};


//-----------------------------------------------------------------------------------------------
enum EPropertySetResult
{
	PSR_SUCCESS,
	PSR_FAIL_TYPE_MISMATCH
};


//-----------------------------------------------------------------------------------------------
enum EDataType
{
	DATATYPE_INT,
	DATATYPE_FLOAT,
	DATATYPE_COLOR,
	DATATYPE_VEC2,
	NUM_DATATYPES,
	DATATYPE_UNSPECIFIED = NUM_DATATYPES
};


//Dummy parent struct for named properties-------------------------------------------------------
struct QuNamedPropertyBase 
{
	virtual ~QuNamedPropertyBase() {}

	EDataType dataType;
};


//-----------------------------------------------------------------------------------------------
template<typename T>
struct TNamedProperty : QuNamedPropertyBase
{
public:
	virtual ~TNamedProperty() override {}
	TNamedProperty(const T& datum) : m_datum(datum) {}

public:
	T m_datum;
};


//-----------------------------------------------------------------------------------------------
class QuNamedProperties
{
public:
	//-----------------------------------------------------------------------------------------------
	//CTORS AND DTORS
	QuNamedProperties() : m_properties() {}
	template<typename... T_Params>
	QuNamedProperties(T_Params... props);
	~QuNamedProperties();
	//-----------------------------------------------------------------------------------------------

public:
	//-----------------------------------------------------------------------------------------------
	//GETTERS AND SETTERS
	template<typename T>
	EPropertyGetResult Get(const QuString& name, T& outDatum, EDataType* outDataType = nullptr) const;
	template<typename T>
	inline EPropertySetResult Set(const QuString& name, const T& datum, EDataType dataType = DATATYPE_UNSPECIFIED);
	void Remove(const QuString& name);
	//-----------------------------------------------------------------------------------------------

public:
	//-----------------------------------------------------------------------------------------------
	//UTILITY FUNCTIONS
	template<typename T>
	void ConsumeParams(const QuString& propName, const T& propValue);
	template<typename T, typename... T_Params>
	void ConsumeParams(const QuString& propName, const T& propValue, T_Params... props);
	//-----------------------------------------------------------------------------------------------

private:
	std::map<QuHash, QuNamedPropertyBase*> m_properties;
};


//-----------------------------------------------------------------------------------------------
class QuSubscriberBase
{
public:
	virtual ~QuSubscriberBase() {}
	virtual void Execute(QuNamedProperties& params) = 0;
};


//-----------------------------------------------------------------------------------------------
template<typename T>
using TMethodFunc = void (T::*)(QuNamedProperties&);


//-----------------------------------------------------------------------------------------------
template<typename T>
class TSubscriber : QuSubscriberBase
{
public:
	virtual ~TSubscriber() override {}
	TSubscriber(T* obj, TMethodFunc<T> method) : m_object(obj), m_method(method) {}
	virtual void Execute(QuNamedProperties& params) override;

private:
	T* m_object;
	TMethodFunc<T> m_method;
};


//-----------------------------------------------------------------------------------------------
typedef void(*EventFunc)(QuNamedProperties& params);


//-----------------------------------------------------------------------------------------------
class QuEventSystem
{
public:
	//-----------------------------------------------------------------------------------------------
	//CTORS AND DTORS
	~QuEventSystem();
	//-----------------------------------------------------------------------------------------------

public:
	//-----------------------------------------------------------------------------------------------
	//UTILITY FUNCTIONS
	static QuEventSystem* GetInstance();
	static void DestroySystem();
	void Fire(const QuString& name, QuNamedProperties& params);
	void Register(const QuString& name, EventFunc freeFunc);
	void Register(const QuString& name, QuSubscriberBase* subscriber);
	void UnregisterFunc(const QuString& name, EventFunc toUnregister);
	void UnregisterFromAll(EventFunc toUnregister);
	template<typename T>
	void UnregisterObject(const QuString& name, const T* object);
	template<typename T>
	void UnregisterFromAll(const T* object);
	//-----------------------------------------------------------------------------------------------

private:
	std::map<QuHash, std::vector<EventFunc>> m_freeFuncSubscribers;
	std::map<QuHash, std::vector<QuSubscriberBase*>> m_objectSubscribers;
	static QuEventSystem* s_instance;
};


//-----------------------------------------------------------------------------------------------
namespace QuEvent
{
	void Fire(const QuString& name);
	void Fire(const QuString& name, QuNamedProperties& params);
	template<typename T, typename... T_Params>
	void Fire(const QuString& name, const QuString& param1Name, const T& param1Value, T_Params... params);

	void Register(const QuString& name, EventFunc freeFunc);
	template<typename T>
	void Register(const QuString& name, T* obj, TMethodFunc<T> method);

	void UnregisterFunc(const QuString& name, EventFunc toUnregister);
	void UnregisterFromAll(EventFunc toUnregister);

	template<typename T>
	void UnregisterObject(const QuString& name, const T* object);

	template<typename T>
	void UnregisterFromAll(const T* object);
}

#include "Quantum/Core/EventSystem.inl"
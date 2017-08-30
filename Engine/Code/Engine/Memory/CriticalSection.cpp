#include "Engine/Memory/CriticalSection.hpp"


//-----------------------------------------------------------------------------------------------
CriticalSection::CriticalSection()
{
	InitializeCriticalSection(&m_cs);
}


//-----------------------------------------------------------------------------------------------
void CriticalSection::Enter()
{
	EnterCriticalSection(&m_cs);
}


//-----------------------------------------------------------------------------------------------
void CriticalSection::Leave()
{
	LeaveCriticalSection(&m_cs);
}


//-----------------------------------------------------------------------------------------------
bool CriticalSection::TryEnter()
{
	return TryEnterCriticalSection(&m_cs) != 0;
}
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//-----------------------------------------------------------------------------------------------
class CriticalSection
{
public:
	CriticalSection();
	void Enter();
	void Leave();
	bool TryEnter();

private:
	CRITICAL_SECTION m_cs;
};


//-----------------------------------------------------------------------------------------------
class CriticalSectionGuard
{
public:
	CriticalSectionGuard(CriticalSection* cs)
		: m_cs(cs)
	{
		m_cs->Enter();
	}
	~CriticalSectionGuard()
	{
		m_cs->Leave();
	}


private:
	CriticalSection* m_cs;
};
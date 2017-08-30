#include "Engine/Core/Clock.hpp"


//-----------------------------------------------------------------------------------------------
void Clock::Update(float deltaSeconds)
{
	float effectiveDeltaSeconds = deltaSeconds * m_timeMultiplier;
	m_deltaTime = effectiveDeltaSeconds;
	m_totalTime += effectiveDeltaSeconds;

	for (Clock* c : m_childClocks)
	{
		c->Update(effectiveDeltaSeconds);
	}
}


//-----------------------------------------------------------------------------------------------
STATIC Clock* Clock::Create(float multiplier, Clock* parent)
{
	Clock* result = new Clock();
	result->m_parent = parent;
	result->m_timeMultiplier = multiplier;
	result->m_totalTime = 0.f;
	result->m_deltaTime = 0.f;

	return result;
}


//-----------------------------------------------------------------------------------------------
STATIC void Clock::Destroy(Clock* toDestroy)
{
	for (Clock* c : toDestroy->m_childClocks)
	{
		Destroy(c);
	}

	delete toDestroy;
}
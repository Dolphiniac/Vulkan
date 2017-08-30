#pragma once

#include <vector>


//-----------------------------------------------------------------------------------------------
class Clock
{
public:
	void AddChild(Clock* toAdd) { m_childClocks.push_back(toAdd); }
	void Update(float deltaSeconds);
	static Clock* Create(float multiplier, Clock* parent);
	static void Destroy(Clock* toDestroy);
	float GetTime() const { return m_totalTime; }
	float GetDeltaTime() const { return m_deltaTime; }

private:
	Clock() = default;
	Clock(const Clock& toCopy) = delete;
	void operator=(const Clock& toCopy) = delete;
	Clock* m_parent;
	std::vector<Clock*> m_childClocks;
	float m_timeMultiplier;
	float m_totalTime;
	float m_deltaTime;
};
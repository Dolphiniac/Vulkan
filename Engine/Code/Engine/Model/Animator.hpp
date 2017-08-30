#pragma once

#include "Engine/Model/AnimationGraph.hpp"
#include <string>


//-----------------------------------------------------------------------------------------------
class Animator
{
public:
	void SetFloat(const std::string& name, const float value);
	void SetBool(const std::string& name, const bool value);
	void SetTrigger(const std::string& name);
	void Tick(float deltaSeconds);
	void SetSkeleton(class Skeleton* skeleton) { m_skeleton = skeleton; }

public:
	class AnimationState* m_currState;
	class AnimationTransition* m_currTransition;
	float m_timeIntoTransition;
	float m_currentNormalizedTime;
	float m_currentDstNormalizedTime;

private:
	void SetTransition(AnimationTransition* transition);
	bool AdvanceIntoTransition(float deltaSeconds);
	void ApplyMatricesToSkeleton(class Matrix44* matrices, int numMatrices);
	void AdvanceIntoState(float deltaSeconds);

private:
	class Skeleton* m_skeleton;
};
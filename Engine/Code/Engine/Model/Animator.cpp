#include "Engine/Model/Animator.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Model/Skeleton.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
void Animator::SetFloat(const std::string& name, const float value)
{
	if (m_currTransition)
	{
		return;
	}

	for (AnimationTransition* transition : m_currState->m_transitions)
	{
		if (transition->m_name != name)
		{
			continue;
		}
		switch (transition->m_type)
		{
		case TRANSITION_THRESHOLD_EQUAL_TO:
			if (value == transition->m_floatVal)
			{
				SetTransition(transition);
				return;
			}
			break;
		case TRANSITION_THRESHOLD_GREATER_THAN:
			if (value > transition->m_floatVal)
			{
				SetTransition(transition);
				return;
			}
			break;
		case TRANSITION_THRESHOLD_LESS_THAN:
			if (value < transition->m_floatVal)
			{
				SetTransition(transition);
				return;
			}
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Animator::SetBool(const std::string& name, const bool value)
{
	if (m_currTransition)
	{
		return;
	}

	for (AnimationTransition* transition : m_currState->m_transitions)
	{
		if (transition->m_name != name)
		{
			continue;
		}

		if (transition->m_type != TRANSITION_BOOL)
		{
			continue;
		}

		if (value == transition->m_boolVal)
		{
			SetTransition(transition);
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Animator::SetTrigger(const std::string& name)
{
	if (m_currTransition)
	{
		return;
	}

	for (AnimationTransition* transition : m_currState->m_transitions)
	{
		if (transition->m_name != name)
		{
			continue;
		}

		if (transition->m_type == TRANSITION_TRIGGER)
		{
			SetTransition(transition);
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Animator::Tick(float deltaSeconds)
{
	if (m_currTransition)
	{
		if (AdvanceIntoTransition(deltaSeconds))
		{
			return;
		}
	}

	AdvanceIntoState(deltaSeconds);
}


//-----------------------------------------------------------------------------------------------
void Animator::SetTransition(AnimationTransition* transition)
{
	m_currTransition = transition;
	m_timeIntoTransition = 0.f;
	if (transition->m_startDstFromBeginning)
	{
		m_currentDstNormalizedTime = 0.f;
	}
	else
	{
		m_currentDstNormalizedTime = m_currentNormalizedTime;
	}
}


//-----------------------------------------------------------------------------------------------
static void CorrectNormalizedTime(float& normalizedTime, bool* didReset = nullptr)
{
	bool reset = false;
	while (normalizedTime >= 1.f)
	{
		normalizedTime -= 1.f;
		reset = true;
	}
	if (didReset)
	{
		*didReset = reset;
	}
}


//-----------------------------------------------------------------------------------------------
bool Animator::AdvanceIntoTransition(float deltaSeconds)
{
	m_timeIntoTransition += deltaSeconds;
	if (m_timeIntoTransition >= m_currTransition->m_blendSeconds)
	{
		m_currentNormalizedTime = m_currentDstNormalizedTime;
		//If no blending, we assume we start the new animation from its beginning
		if (m_currTransition->m_blendSeconds == 0.f)
		{
			m_currentNormalizedTime = 0.f;
		}
		m_currState = m_currTransition->m_dstState;
		m_currTransition = nullptr;
		return false;
	}

	float dstWeight = m_timeIntoTransition / m_currTransition->m_blendSeconds;
	float srcWeight = 1.f - dstWeight;
	float srcDivisor = 1.f / m_currTransition->m_srcState->GetAnimationLength();
	float dstDivisor = 1.f / m_currTransition->m_dstState->GetAnimationLength();

	float deltaNormalizedTime = deltaSeconds * (srcDivisor * srcWeight + dstDivisor * dstWeight);
	m_currentNormalizedTime += deltaNormalizedTime;
	m_currentDstNormalizedTime += deltaNormalizedTime;
	CorrectNormalizedTime(m_currentNormalizedTime);
	CorrectNormalizedTime(m_currentDstNormalizedTime);
	std::vector<Matrix44> srcMatrices = m_currTransition->m_srcState->GetMatricesForSkeletonAtNormalizedTime(m_skeleton, m_currentNormalizedTime);
	std::vector<Matrix44> dstMatrices = m_currTransition->m_dstState->GetMatricesForSkeletonAtNormalizedTime(m_skeleton, m_currentDstNormalizedTime);

	std::vector<Matrix44> compositeMatrices;
	compositeMatrices.reserve(srcMatrices.size());
	for (size_t i = 0; i < srcMatrices.size(); i++)
	{
		Matrix44 compositeMatrix = srcMatrices[i] * srcWeight + dstMatrices[i] * dstWeight;
		compositeMatrices.push_back(compositeMatrix);
	}

	ApplyMatricesToSkeleton(&compositeMatrices[0], compositeMatrices.size());

	return true;
}


//-----------------------------------------------------------------------------------------------
void Animator::AdvanceIntoState(float deltaSeconds)
{
	m_currentNormalizedTime += deltaSeconds / m_currState->GetAnimationLength();

	AnimationTransition* animationFinishTransition = nullptr;
	for (AnimationTransition* transition : m_currState->m_transitions)
	{
		if (transition->m_type == TRANSITION_ANIMATION_COMPLETE)
		{
			animationFinishTransition = transition;
			break;
		}
	}

	float normalizedTimeThresholdForAnimationFinishBlend = 0.f;
	if (animationFinishTransition)
	{
		normalizedTimeThresholdForAnimationFinishBlend = 1.f - animationFinishTransition->m_blendSeconds / m_currState->GetAnimationLength();
	}

	bool didReset;
	CorrectNormalizedTime(m_currentNormalizedTime, &didReset);

	if (animationFinishTransition)
	{
		if (didReset || m_currentNormalizedTime >= normalizedTimeThresholdForAnimationFinishBlend)
		{
			SetTransition(animationFinishTransition);
		}
	}

	std::vector<Matrix44> transformationMatrices = m_currState->GetMatricesForSkeletonAtNormalizedTime(m_skeleton, m_currentNormalizedTime);
	ApplyMatricesToSkeleton(&transformationMatrices[0], transformationMatrices.size());
}


//-----------------------------------------------------------------------------------------------
void Animator::ApplyMatricesToSkeleton(class Matrix44* matrices, int numMatrices)
{
	for (int i = 0; i < numMatrices; i++)
	{
		m_skeleton->SetWorldTransformForJoint(i, matrices[i]);
	}
}
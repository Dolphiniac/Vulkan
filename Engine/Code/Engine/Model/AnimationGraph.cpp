#include "Engine/Model/AnimationGraph.hpp"
#include "Engine/Model/Motion.hpp"
#include "Engine/Math/Matrix44.hpp"


//-----------------------------------------------------------------------------------------------
std::vector<Matrix44> AnimationState::GetMatricesForSkeletonAtNormalizedTime(class Skeleton* skeleton, float normalizedTime)
{
	return m_motion->GetMatricesForSkeletonAtNormalizedTime(skeleton, normalizedTime);
}


//-----------------------------------------------------------------------------------------------
float AnimationState::GetAnimationLength()
{
	return m_motion->m_totalLengthOfAnimation + m_motion->startTime;
}


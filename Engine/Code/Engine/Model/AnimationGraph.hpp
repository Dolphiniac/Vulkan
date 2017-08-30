#pragma once

#include <vector>


//-----------------------------------------------------------------------------------------------
class AnimationGraph
{
public:
	class AnimationState* m_entryState;
};


//-----------------------------------------------------------------------------------------------
class AnimationState
{
public:
	std::vector<class Matrix44> GetMatricesForSkeletonAtNormalizedTime(class Skeleton* skeleton, float normalizedTime);
	float GetAnimationLength();

public:
	class Motion* m_motion;
	std::vector<class AnimationTransition*> m_transitions;
};


//-----------------------------------------------------------------------------------------------
enum ETransitionType
{
	TRANSITION_TRIGGER,
	TRANSITION_BOOL,
	TRANSITION_THRESHOLD_GREATER_THAN,
	TRANSITION_THRESHOLD_LESS_THAN,
	TRANSITION_THRESHOLD_EQUAL_TO,
	TRANSITION_ANIMATION_COMPLETE
};


//-----------------------------------------------------------------------------------------------
class AnimationTransition
{
public:
	AnimationState* m_srcState;
	AnimationState* m_dstState;
	float m_blendSeconds;
	ETransitionType m_type;
	union
	{
		float m_floatVal;
		bool m_boolVal;
	};
	bool m_startDstFromBeginning;
	std::string m_name;
};
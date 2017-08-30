#pragma once

#include <vector>


//-----------------------------------------------------------------------------------------------
// class Motion
// {
// public:
// 	Motion(int jointCount, class Matrix44* keyFrames, float totalSeconds, float frameRate);
// 	size_t m_frameCount;
// 	float m_totalLengthSeconds;
// 	float m_frameRate;
// 	float m_frameTime;
// 	int m_jointCount;
// 	class Matrix44* m_keyFrames;
// 
// 	void GetFrameIndicesWithBlend(size_t& outFrameIndex0, size_t& outFrameIndex1, float& outBlend, float inTime);
// 	void ApplyMotionToSkeleton(class Skeleton* skeleton, float time);
// };

enum EPlayMode
{
	CLAMP,
	LOOPING,
	PINGPONG
};


extern class Motion* loadedMotion;

struct AnimationEvent
{
	std::string name;
	float time;
};


//-----------------------------------------------------------------------------------------------
class Motion
{
	friend class AnimationState;
	static const int MOTION_VERSION = 1;
public:
	Motion(float lengthOfAnimation) : m_totalLengthOfAnimation(lengthOfAnimation), m_totalTime(0.f), m_mode(LOOPING), m_isPlaying(true) {}
	void SetPlayMode(EPlayMode mode) { m_mode = mode; }
	void ResetAnimation() { m_totalTime = 0.f; }

	//Broken in favor of Animator-centric paradigm
	void Update(class Skeleton* skeleton, float deltaSeconds);
	void AddAnimationCurve(class AnimationCurve* curve) { m_curves.push_back(curve); }
	void WriteToFile(const std::string& filepath);
	void Play() { m_isPlaying = true; }
	void Pause() { m_isPlaying = false; }
	static Motion* ReadFromFile(const std::string& filepath);
	~Motion();

public:
	float m_totalLengthOfAnimation;
	float startTime;
	std::vector<class AnimationCurve*> m_curves;
	std::vector<AnimationEvent> m_events;
	std::string name;

private:
	float m_totalTime;
	EPlayMode m_mode;
	int m_version = MOTION_VERSION;
	bool m_isPlaying;

private:
	std::vector<class Matrix44> GetMatricesForSkeletonAtNormalizedTime(class Skeleton* skeleton, float normalizedTime);
};
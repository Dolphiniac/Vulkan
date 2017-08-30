#pragma once

#include <vector>

enum ECurveInterpolationType
{
	CONSTANT,
	LINEAR,
	CUBIC
};


//-----------------------------------------------------------------------------------------------
template<typename T>
struct TKeyFrame
{
	float timeIntoAnimation;
	EInterpolationType type;
	T transformationChannelValue;
	T leftSlope;
	T rightSlope;
};


//-----------------------------------------------------------------------------------------------
enum EExtrapolationMode
{
	EXTRAPOLATION_CLAMP,
	EXTRAPOLATION_LOOP,
	EXTRAPOLATION_PINGPONG
};


//-----------------------------------------------------------------------------------------------
template<typename T>
class TKeyframeProperty
{
public:
	T EvaluateAt(EExtrapolationMode extrapolationMode, float parametric) const;
	void InsertKeyFrame(const KeyFrame<T>& frame) { m_keyFrames.push_back(frame); }
	void GetBestKeyFrames(EExtrapolationMode extrapolationMode, KeyFrame<T>& outFrame1, KeyFrame<T>& outFrame2, float& outBlend, float parametric) const;
	static T LerpOnType(EInterpolationType type, T leftVal, T rightVal, T leftSlope, T rightSlope, float blend);

private:
	std::vector<KeyFrame<T>> m_keyFrames;
};


//-----------------------------------------------------------------------------------------------
template<typename T>
class TAnimatedProperty
{
public:
	void Tick(float deltaSeconds) { m_currentTime += deltaSeconds; }
	T Get() const;

private:
	TKeyframeProperty<T> m_property;
	float m_currentTime;
	float m_totalTime;
	EExtrapolationMode m_extrapolationMode;
};

#include "Engine/UI/KeyframeProperty.inl"
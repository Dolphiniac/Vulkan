#pragma once

#include "Engine/Math/Matrix44.hpp"
#include "Engine/Model/Skeleton.hpp"

#include <vector>

enum EInterpolationType
{
	CONSTANT,
	LINEAR,
	CUBIC
};


//-----------------------------------------------------------------------------------------------
struct KeyFrame
{
	float timeIntoAnimation;
	EInterpolationType type;
	float transformationChannelValue;
	float leftSlope;
	float rightSlope;
};


//-----------------------------------------------------------------------------------------------
class TransformationCurve
{
public:
	float GetTransformationAt(float time, bool isRotation = false);
	void InsertKeyFrame(const KeyFrame& frame) { m_keyFrames.push_back(frame); }
	void GetBestKeyFrames(KeyFrame& outFrame1, KeyFrame& outFrame2, float& outBlend, float time);
	static float LerpOnType(EInterpolationType type, float leftVal, float rightVal, float leftSlope, float rightSlope, float blend);
	void SaveToWriter(class BinaryWriter& writer) const;
	static TransformationCurve CreateFromReader(class BinaryReader& reader);

private:
	std::vector<KeyFrame> m_keyFrames;
};


//-----------------------------------------------------------------------------------------------
class AnimationCurve
{
public:
	Matrix44 EvaluateLocalTransformAt(float time, const JointMeta& meta);
	void SaveToWriter(class BinaryWriter& writer) const;
	static AnimationCurve* CreateFromReader(class BinaryReader& reader);

public:
	TransformationCurve m_scaling[3];
	TransformationCurve m_rotation[3];
	TransformationCurve m_translation[3];
	Matrix44 m_transformationInverse;
};
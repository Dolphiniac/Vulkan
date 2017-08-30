#include "Engine/Model/AnimationCurve.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/BinaryReader.hpp"
#include "Engine/Core/BinaryWriter.hpp"


//-----------------------------------------------------------------------------------------------
Matrix44 AnimationCurve::EvaluateLocalTransformAt(float time, const JointMeta& meta)
{
	float scaleX = m_scaling[0].GetTransformationAt(time);
	float scaleY = m_scaling[1].GetTransformationAt(time);
	float scaleZ = m_scaling[2].GetTransformationAt(time);
	Vector3 scaling(scaleX, scaleY, scaleZ);
	float rotX = m_rotation[0].GetTransformationAt(time, true);
	float rotY = m_rotation[1].GetTransformationAt(time, true);
	float rotZ = m_rotation[2].GetTransformationAt(time, true);
	Vector3 rotation(rotX, rotY, rotZ);
	float transX = m_translation[0].GetTransformationAt(time);
	float transY = m_translation[1].GetTransformationAt(time);
	float transZ = m_translation[2].GetTransformationAt(time);
	Vector3 translation(transX, transY, transZ);

	Matrix44 matX;
	matX.MakeRotationX(rotX);
	Matrix44 matY;
	matY.MakeRotationY(rotY);
	matY.Invert();
	Matrix44 matZ;
	matZ.MakeRotationZ(rotZ);
	Matrix44 rot = matX * matY * matZ;

	return m_transformationInverse * meta.CalculateFinalLocal(translation, rot, scaling);
}


//-----------------------------------------------------------------------------------------------
void TransformationCurve::GetBestKeyFrames(KeyFrame& outFrame1, KeyFrame& outFrame2, float& outBlend, float time)
{
	for (size_t i = 0; i < m_keyFrames.size(); i++)
	{
		if (m_keyFrames[i].timeIntoAnimation == time)
		{
			outFrame2 = m_keyFrames[i];
			outFrame1 = m_keyFrames[i];
			outBlend = 0.f;
			return;
		}
		if (m_keyFrames[i].timeIntoAnimation > time)
		{
			outFrame2 = m_keyFrames[i];
			if (i == 0)
			{
				outFrame1 = m_keyFrames[i];
			}
			else
			{
				outFrame1 = m_keyFrames[i - 1];
			}
			outBlend = RangeMap(time, outFrame1.timeIntoAnimation, outFrame2.timeIntoAnimation, 0.f, 1.f);
			return;
		}
	}
	outFrame1 = outFrame2 = m_keyFrames[0];
	outBlend = 0.f;
	//ERROR_RECOVERABLE("Should have found a good keyframe pair.  Is your time between 0 and total animation time?");
}


//-----------------------------------------------------------------------------------------------
static float CubicSplineInterpolate(float leftVal, float rightVal, float leftSlope, float rightSlope, float interpolant)
{
// 
// 	float squareInterpolant = interpolant * interpolant;
// 	float cubeInterpolant = squareInterpolant * interpolant;
// 	float term0 = leftVal * (2.f * cubeInterpolant - 3.f * squareInterpolant + 1);
// 	float term1 = leftSlope * (cubeInterpolant - 2.f * squareInterpolant + interpolant);
// 	float term2 = rightVal * (-2.f * cubeInterpolant + 3.f * squareInterpolant);
// 	float term3 = rightSlope * (cubeInterpolant - squareInterpolant);
// 
// 	return term0 + term1 + term2 + term3;

	float t = interpolant;
	float a = leftSlope - (rightVal - leftVal);
	float b = -rightSlope + (rightVal - leftVal);

	return (1 - t) * leftVal + t * rightVal + t * (1 - t) * (a * (1 - t) + b * t);
}


//-----------------------------------------------------------------------------------------------
float TransformationCurve::LerpOnType(EInterpolationType type, float leftVal, float rightVal, float leftSlope, float rightSlope, float blend)
{
	switch (type)
	{
	case CONSTANT:
		return leftVal;
		break;
	case LINEAR:
		return Lerp(leftVal, rightVal, blend);
		break;
	case CUBIC:
		return CubicSplineInterpolate(leftVal, rightVal, leftSlope, rightSlope, blend);
		break;
	default:
		ERROR_AND_DIE("Keyframe must have valid interpolation type");
	}
}


//-----------------------------------------------------------------------------------------------
float TransformationCurve::GetTransformationAt(float time, bool isRotation)
{
	KeyFrame frame1, frame2;
	float blend;
	GetBestKeyFrames(frame1, frame2, blend, time);
	if (isRotation)
	{
		FindShortestJourneyBetween(frame1.transformationChannelValue, frame2.transformationChannelValue);
	}
	return LerpOnType(frame1.type, frame1.transformationChannelValue, frame2.transformationChannelValue, frame1.leftSlope, frame1.rightSlope, blend);
	
}


//-----------------------------------------------------------------------------------------------
void AnimationCurve::SaveToWriter(class BinaryWriter& writer) const
{
	writer.Write(m_transformationInverse);
	for (int i = 0; i < 3; i++)
	{
		m_scaling[i].SaveToWriter(writer);
		m_rotation[i].SaveToWriter(writer);
		m_translation[i].SaveToWriter(writer);
	}
}


//-----------------------------------------------------------------------------------------------
AnimationCurve* AnimationCurve::CreateFromReader(class BinaryReader& reader)
{
	AnimationCurve* result = new AnimationCurve();
	reader.Read(result->m_transformationInverse);

	for (int i = 0; i < 3; i++)
	{
		result->m_scaling[i] = TransformationCurve::CreateFromReader(reader);
		result->m_rotation[i] = TransformationCurve::CreateFromReader(reader);
		result->m_translation[i] = TransformationCurve::CreateFromReader(reader);
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
void TransformationCurve::SaveToWriter(class BinaryWriter& writer) const
{
	writer.Write((int)m_keyFrames.size());
	for (const KeyFrame& keyframe : m_keyFrames)
	{
		writer.Write(keyframe.timeIntoAnimation);
		writer.Write((int)keyframe.type);
		writer.Write(keyframe.transformationChannelValue);
		if (keyframe.type == CUBIC)
		{
			writer.Write(keyframe.leftSlope);
			writer.Write(keyframe.rightSlope);
		}
	}
}


//-----------------------------------------------------------------------------------------------
TransformationCurve TransformationCurve::CreateFromReader(class BinaryReader& reader)
{
	TransformationCurve result;
	int numKeyframes;
	reader.Read(numKeyframes);

	for (int i = 0; i < numKeyframes; i++)
	{
		KeyFrame frame;
		reader.Read(frame.timeIntoAnimation);
		reader.Read((int&)frame.type);
		reader.Read(frame.transformationChannelValue);
		if (frame.type == CUBIC)
		{
			reader.Read(frame.leftSlope);
			reader.Read(frame.rightSlope);
		}
		result.InsertKeyFrame(frame);
	}

	return result;
}
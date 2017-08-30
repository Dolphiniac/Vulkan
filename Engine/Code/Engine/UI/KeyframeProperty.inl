//-----------------------------------------------------------------------------------------------
// INLINE FILE FOR KEYFRAMEPROPERTY.HPP
//-----------------------------------------------------------------------------------------------

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
template<typename T>
T TKeyframeProperty<T>::EvaluateAt(EExtrapolationMode extrapolationMode, float parametric) const
{
	KeyFrame<T> frame1, frame2;
	float blend;
	GetBestKeyFrames(extrapolationMode, frame1, frame2, blend, parametric);
	return LerpOnType(frame1.type, frame1.transformationChannelValue, frame2.transformationChannelValue, frame1.leftSlope, frame1.rightSlope, blend);
}


//-----------------------------------------------------------------------------------------------
static void WrapParametricUsingExtrapolation(EExtrapolationMode mode, float& parametric)
{
	if (parametric < 0.f)
	{
		switch (mode)
		{
		case EXTRAPOLATION_CLAMP:
			parametric = 0.f;
			break;
		case EXTRAPOLATION_LOOP:
			while (parametric < 0.f)
			{
				parametric += 1.f;
			}
			break;
		case EXTRAPOLATION_PINGPONG:
			int counter = 0;
			while (parametric < 0.f)
			{
				parametric += 1.f;
				counter++;
			}
			if ((counter & 1) == 1)
			{
				parametric = 1.f - parametric;
			}
			break;
		}
	}
	else if (parametric > 1.f)
	{
		switch (mode)
		{
		case EXTRAPOLATION_CLAMP:
			parametric = 1.f;
			break;
		case EXTRAPOLATION_LOOP:
			while (parametric > 1.f)
			{
				parametric -= 1.f;
			}
			break;
		case EXTRAPOLATION_PINGPONG:
			int counter = 0;
			while (parametric > 1.f)
			{
				parametric -= 1.f;
				counter++;
			}
			if ((counter & 1) == 1)
			{
				parametric = 1.f - parametric;
			}
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
template<typename T>
void TKeyframeProperty<T>::GetBestKeyFrames(EExtrapolationMode extrapolationMode, KeyFrame<T>& outFrame1, KeyFrame<T>& outFrame2, float& outBlend, float parametric) const
{
	WrapParametricUsingExtrapolation(extrapolationMode, parametric);
	for (size_t i = 0; i < m_keyFrames.size(); i++)
	{
		if (m_keyFrames[i].timeIntoAnimation == parametric)
		{
			outFrame2 = m_keyFrames[i];
			outFrame1 = m_keyFrames[i];
			outBlend = 0.f;
			return;
		}
		if (m_keyFrames[i].timeIntoAnimation > parametric)
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
			outBlend = RangeMap(parametric, outFrame1.timeIntoAnimation, outFrame2.timeIntoAnimation, 0.f, 1.f);
			return;
		}
	}
	outFrame1 = outFrame2 = m_keyFrames[0];
	outBlend = 0.f;
}


//-----------------------------------------------------------------------------------------------
static float CubicSplineInterpolate(float leftVal, float rightVal, float leftSlope, float rightSlope, float interpolant)
{
	float t = interpolant;
	float a = leftSlope - (rightVal - leftVal);
	float b = -rightSlope + (rightVal - leftVal);

	return (1 - t) * leftVal + t * rightVal + t * (1 - t) * (a * (1 - t) + b * t);
}


//-----------------------------------------------------------------------------------------------
template<typename T>
T TKeyframeProperty<T>::LerpOnType(EInterpolationType type, T leftVal, T rightVal, T leftSlope, T rightSlope, float blend)
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
		ERROR_AND_DIE("Cannot interpolate cubic spline on non-float type");
		break;
	default:
		ERROR_AND_DIE("Keyframe must have valid interpolation type");
	}
}


//-----------------------------------------------------------------------------------------------
template<> float TKeyframeProperty::LerpOnType(EInterpolationType type, float leftVal, float rightVal, float leftSlope, float rightSlope, float blend)
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
template<typename T>
T TAnimatedProperty<T>::Get() const
{
	float parametric = RangeMap(m_currentTime, 0.f, m_totalTime, 0.f, 1.f);
	return m_property.EvaluateAt(m_extrapolationMode, parametric);
}
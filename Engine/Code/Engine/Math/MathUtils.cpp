#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Matrix44.hpp"
#include <math.h>


//-----------------------------------------------------------------------------------------------
bool DoDiscsOverlap(Vector2 center1, float radius1, Vector2 center2, float radius2)
{
	Vector2 displacement = center2 - center1;
	float distanceSquared = displacement.LengthSquared();

	return distanceSquared < (radius1 + radius2) * (radius1 + radius2);
}


//-----------------------------------------------------------------------------------------------
float RangeMap(float toMap, float inputStart, float inputEnd, float outputStart, float outputEnd)
{
	return (toMap - inputStart) / (inputEnd - inputStart) * (outputEnd - outputStart) + outputStart;
}


//-----------------------------------------------------------------------------------------------
float Clampf(float inValue, float min, float max)
{
	if (inValue < min)
	{
		return min;
	}
	else if (inValue > max)
	{
		return max;
	}
	else
	{
		return inValue;
	}
}


//-----------------------------------------------------------------------------------------------
int Clampi(int inValue, int min, int max)
{
	if (inValue < min)
	{
		return min;
	}
	else if (inValue > max)
	{
		return max;
	}
	else
	{
		return inValue;
	}
}


//-----------------------------------------------------------------------------------------------
int Lerp(int startVal, int endVal, float time)
{
	float alpha = Clampf(time, 0.f, 1.f);
	float result = RangeMap(alpha, 0.f, 1.f, (float)startVal, (float)endVal);
	return (int)result;
}


//-----------------------------------------------------------------------------------------------
float Lerp(float startVal, float endVal, float time)
{
	float result = Clampf(time, 0.f, 1.f);
	result = RangeMap(result, 0.f, 1.f, startVal, endVal);
	return result;
}

//-----------------------------------------------------------------------------------------------
float GetShortestRotationPath(float startOrientation, float endOrientation)
{
	float distance = endOrientation - startOrientation;
	while (distance > 180.f)
	{
		distance -= 360.f;
	}

	while (distance < -180.f)
	{
		distance += 360.f;
	}

	return distance;
}


//-----------------------------------------------------------------------------------------------
void FindShortestJourneyBetween(float startOrientation, float& endOrientation)
{
	float distance = GetShortestRotationPath(startOrientation, endOrientation);

	endOrientation = startOrientation + distance;
}


//-----------------------------------------------------------------------------------------------
float Min(float a, float b)
{
	return (a > b) ? b : a;
}


//-----------------------------------------------------------------------------------------------
float Max(float a, float b)
{
	return (a > b) ? a : b;
}


//-----------------------------------------------------------------------------------------------
unsigned int Max(unsigned int a, unsigned int b)
{
	return (a > b) ? a : b;
}


//-----------------------------------------------------------------------------------------------
float CosDegrees(float degrees)
{
	return (float)cos(degrees * DEG2RAD);
}


//-----------------------------------------------------------------------------------------------
float SinDegrees(float degrees)
{
	return (float)sin(degrees * DEG2RAD);
}


//-----------------------------------------------------------------------------------------------
float SmoothStep(float inputZeroToOne)
{
	float inputSquared = inputZeroToOne * inputZeroToOne;
	return (3.f * inputSquared) - (2.f * inputSquared * inputZeroToOne);
}


//-----------------------------------------------------------------------------------------------
bool PointInsideBox(const Vector2& point, const AABB2& box)
{
	if (point.x < box.mins.x)
		return false;

	if (point.y < box.mins.y)
		return false;

	if (point.x > box.maxs.x)
		return false;

	if (point.y > box.maxs.y)
		return false;

	return true;
}


//-----------------------------------------------------------------------------------------------
Vector3 Lerp(const Vector3& startVec, const Vector3& endVec, float time)
{
	float x = Lerp(startVec.x, endVec.x, time);
	float y = Lerp(startVec.y, endVec.y, time);
	float z = Lerp(startVec.z, endVec.z, time);

	return Vector3(x, y, z);
}


//-----------------------------------------------------------------------------------------------
Vector2 Lerp(const Vector2& startVec, const Vector2& endVec, float time)
{
	Vector3 start(startVec.x, startVec.y, 0.f);
	Vector3 end(endVec.x, endVec.y, 0.f);
	Vector3 result = Lerp(start, end, time);

	return Vector2(result.x, result.y);
}


//-----------------------------------------------------------------------------------------------
//Matrix44 Lerp(const Matrix44& startVal, const Matrix44& endVal, float time)
//{
//	Vector3 r0, u0, f0, t0;
//	startVal.GetBasis(&r0, &u0, &f0, &t0);
//	Vector3 r1, u1, f1, t1;
//	endVal.GetBasis(&r1, &u1, &f1, &t1);
//
//	Vector3 r, u, f, t;
//	r = Slerp(r0, r1, time);
//	u = Slerp(u0, u1, time);
//	f = Slerp(f0, f1, time);
//	t = Lerp(t0, t1, time);
//
//	return Matrix44::MakeFromBasis(r, u, f, t);
//}


//-----------------------------------------------------------------------------------------------
float GetRandominRange(float min, float max)
{
	return RangeMap((float)rand(), 0.f, (float)RAND_MAX, min, max);
}


//-----------------------------------------------------------------------------------------------
float GetRandomNormalized()
{
	return GetRandominRange(0.f, 1.f);
}


//-----------------------------------------------------------------------------------------------
bool AreBoundingBoxesPossiblyOverlapping(const AABB2& box1, const AABB2& box2)
{
	if (box1.mins.x > box2.maxs.x)
	{
		return false;
	} 

	if (box1.mins.y > box2.maxs.y)
	{
		return false;
	}

	if (box1.maxs.x < box2.mins.x)
	{
		return false;
	}

	if (box1.maxs.y < box2.mins.y)
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
void IncrementIndexWrapped(int& index, int totalNumElements)
{
	index++;
	if (index >= totalNumElements)
	{
		index = 0;
	}
}


//-----------------------------------------------------------------------------------------------
void DecrementIndexWrapped(int& index, int totalNumElements)
{
	index--;
	if (index < 0)
	{
		index = totalNumElements - 1;
	}
}
#pragma once
#include "Engine/Math/Vector2.hpp"


//-----------------------------------------------------------------------------------------------
const float pi = 3.1415926535897932384626433832795f;
const float DEG2RAD = pi / 180.f;
const float RAD2DEG = 1.f / DEG2RAD;
const double TWO_PI = 6.283185307179586476925286766559;
const double PI = 3.1415926535897932384626433832795;
const double HALF_PI = 1.5707963267948966192313216916398;
const double QUARTER_PI = 0.78539816339744830961566084581988;
const double SQRT_2 = 1.4142135623730950488016887242097;
const double SQRT_3 = 1.7320508075688772935274463415059;
const double SQRT_2_OVER_2 = 0.70710678118654752440084436210485;
const double SQRT_3_OVER_2 = 0.86602540378443864676372317075294;
const double SQRT_3_OVER_3 = 0.57735026918962576450914878050196;
const double SQRT_3_OVER_6 = 0.28867513459481288225457439025098;

const float fTWO_PI = 6.283185307179586476925286766559f;
const float fPI = 3.1415926535897932384626433832795f;
const float fHALF_PI = 1.5707963267948966192313216916398f;
const float fQUARTER_PI = 0.78539816339744830961566084581988f;
const float fSQRT_2 = 1.4142135623730950488016887242097f;
const float fSQRT_3 = 1.7320508075688772935274463415059f;
const float fSQRT_2_OVER_2 = 0.70710678118654752440084436210485f;
const float fSQRT_3_OVER_2 = 0.86602540378443864676372317075294f;
const float fSQRT_3_OVER_3 = 0.57735026918962576450914878050196f;
const float fSQRT_3_OVER_6 = 0.28867513459481288225457439025098f;


//-----------------------------------------------------------------------------------------------
class AABB2;


//-----------------------------------------------------------------------------------------------
bool DoDiscsOverlap(Vector2 center1, float radius1, Vector2 center2, float radius2);
float RangeMap(float toMap, float inputStart, float inputEnd, float outputStart, float outputEnd);
float Clampf(float inValue, float min, float max);
int Clampi(int inValue, int min, int max);
int Lerp(int startVal, int endVal, float time);
float Lerp(float startVal, float endVal, float time);
Vector3 Lerp(const Vector3& startVec, const Vector3& endVec, float time);
Vector2 Lerp(const Vector2& startVec, const Vector2& endVec, float time);
//Vector3 Slerp(const Vector3& startVec, const Vector3& endVec, float time);
//Matrix44 Lerp(const Matrix44& startMat, const Matrix44& endMat, float time);
float GetShortestRotationPath(float startOrientation, float endOrientation);
void FindShortestJourneyBetween(float startOrientation, float& endOrientation);
float Min(float a, float b);
float Max(float a, float b);
unsigned int Max(unsigned int a, unsigned int b);
float CosDegrees(float degrees);
float SinDegrees(float degrees);
float SmoothStep(float inputZeroToOne);
bool PointInsideBox(const Vector2& point, const AABB2& box);
float GetRandominRange(float min, float max);
float GetRandomNormalized();
bool AreBoundingBoxesPossiblyOverlapping(const AABB2& box1, const AABB2& box2);
void IncrementIndexWrapped(int& index, int totalNumElements);
void DecrementIndexWrapped(int& index, int totalNumElements);


//-----------------------------------------------------------------------------------------------
inline float SmoothStep5(float inputZeroToOne)
{
	const float& t = inputZeroToOne;
	return t * t * t * (t * (t * 6.f - 15.f) + 10.f);
}


//-----------------------------------------------------------------------------------------------
template<typename T>
bool IsBitSet(const T& bitfield, int index)
{
	return ((1 << index) & bitfield) != 0;
}
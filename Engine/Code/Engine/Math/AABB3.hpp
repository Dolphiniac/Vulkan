#pragma once

#include "Engine/Math/Vector3.hpp"


//-----------------------------------------------------------------------------------------------
class AABB3
{
public:
	AABB3();
	AABB3(const Vector3& _mins, const Vector3& _maxs);

public:
	Vector3 mins;
	Vector3 maxs;
};


//-----------------------------------------------------------------------------------------------
inline AABB3::AABB3()
: mins(Vector3::Zero)
, maxs(Vector3::Zero)
{

}


//-----------------------------------------------------------------------------------------------
inline AABB3::AABB3(const Vector3& _mins, const Vector3& _maxs)
: mins(_mins)
, maxs(_maxs)
{

}
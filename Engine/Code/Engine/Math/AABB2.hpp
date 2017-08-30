#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector2.hpp"


//-----------------------------------------------------------------------------------------------
class AABB2
{
public:
	Vector2 mins;
	Vector2 maxs;

public:
	AABB2();
	AABB2(const Vector2& min, const Vector2& max);
	AABB2(float minX, float minY, float maxX, float maxY);
};


//-----------------------------------------------------------------------------------------------
class AABB2i
{
public:
	IntVector2 mins;
	IntVector2 maxs;
};
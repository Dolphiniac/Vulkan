#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------------------------
AABB2::AABB2()
: mins()
, maxs()
{
}


//-----------------------------------------------------------------------------------------------
AABB2::AABB2(const Vector2& min, const Vector2& max)
: mins(min)
, maxs(max)
{
}


//-----------------------------------------------------------------------------------------------
AABB2::AABB2(float minX, float minY, float maxX, float maxY)
: mins(Vector2(minX, minY))
, maxs(Vector2(maxX, maxY))
{

}
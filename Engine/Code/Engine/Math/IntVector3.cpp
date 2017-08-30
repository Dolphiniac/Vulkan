#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"

#include <math.h>

#define STATIC


//-----------------------------------------------------------------------------------------------
STATIC IntVector3 IntVector3::Zero = IntVector3(0, 0, 0);


//-----------------------------------------------------------------------------------------------
IntVector3::IntVector3(const IntVector2& toCopy)
: x(toCopy.x)
, y(toCopy.y)
, z(0)
{

}


//-----------------------------------------------------------------------------------------------
IntVector3::IntVector3(const Vector2& toCopy)
: x((int)floor(toCopy.x))
, y((int)floor(toCopy.y))
, z(0)
{

}


//-----------------------------------------------------------------------------------------------
IntVector3::IntVector3(const Vector3& toCopy)
: x((int)floor(toCopy.x))
, y((int)floor(toCopy.y))
, z((int)floor(toCopy.z))
{

}


//-----------------------------------------------------------------------------------------------
float IntVector3::Length() const
{
	float sqrLength = (float)LengthSquared();
	if (sqrLength == 1.f)
		return 1.f;

	return sqrtf(sqrLength);
}
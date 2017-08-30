#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <math.h>


//-----------------------------------------------------------------------------------------------
Vector2 Vector2::Zero = Vector2(0.f, 0.f);


//-----------------------------------------------------------------------------------------------
Vector2::Vector2(const IntVector2& toCopy)
: x((float)toCopy.x)
, y((float)toCopy.y)
{
}


//-----------------------------------------------------------------------------------------------
Vector2::Vector2(const IntVector3& toCopy)
: x((float)toCopy.x)
, y((float)toCopy.y)
{

}


//-----------------------------------------------------------------------------------------------
Vector2::Vector2(const Vector3& toCopy)
: x(toCopy.x)
, y(toCopy.y)
{

}


//-----------------------------------------------------------------------------------------------
Vector2::Vector2(const std::string& toParse)
{
	std::vector<std::string> pieces = SplitOnDelimiter(toParse, ',');
	if (pieces.size() == 1)
	{
		x = y = std::stof(pieces[0]);
	}
	else if (pieces.size() == 2)
	{
		x = std::stof(pieces[0]);
		y = std::stof(pieces[1]);
	}
	else
	{
		ERROR_AND_DIE(Stringf("Bad string %s, cannot parse to Vector2", toParse.c_str()));
	}
}


//-----------------------------------------------------------------------------------------------
float Vector2::Length()
{
	float sqrLength = LengthSquared();
	if (LengthSquared() == 1.f)
		return 1.f;
	if (LengthSquared() == 0.f)
	{
		return 0.f;
	}

	return sqrtf(sqrLength);
}


//-----------------------------------------------------------------------------------------------
void Vector2::Normalize()
{
	float length = Length();

	if (length == 0.f)
	{
		return;
	}

	if (length == 1.f)
	{
		return;
	}

	float oneOverLength = 1.f / length;

	x *= oneOverLength;
	y *= oneOverLength;
}


//-----------------------------------------------------------------------------------------------
Vector2 Vector2::Normalized() const
{
	Vector2 result(*this);
	result.Normalize();
	return result;
}
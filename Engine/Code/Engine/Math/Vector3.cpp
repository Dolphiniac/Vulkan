#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <math.h>

#define STATIC


//-----------------------------------------------------------------------------------------------
STATIC Vector3 Vector3::Zero = Vector3(0.f, 0.f, 0.f);
STATIC Vector3 Vector3::Right = Vector3(1.f, 0.f, 0.f);
STATIC Vector3 Vector3::Up = Vector3(0.f, 1.f, 0.f);
STATIC Vector3 Vector3::Forward = Vector3(0.f, 0.f, 1.f);


//-----------------------------------------------------------------------------------------------
Vector3::Vector3(const IntVector3& toCopy)
: x((float)toCopy.x)
, y((float)toCopy.y)
, z((float)toCopy.z)
{

}


//-----------------------------------------------------------------------------------------------
Vector3::Vector3(const IntVector2& toCopy)
: x((float)toCopy.x)
, y((float)toCopy.y)
, z(0.f)
{

}


//-----------------------------------------------------------------------------------------------
Vector3::Vector3(const Vector2& toCopy)
: x(toCopy.x)
, y(toCopy.y)
, z(0.f)
{

}


//-----------------------------------------------------------------------------------------------
Vector3::Vector3(const std::string& toParse)
{
	std::vector<std::string> components = SplitOnDelimiter(toParse, ',');
	GUARANTEE_OR_DIE(components.size() == 3, Stringf("String %s cannot be parsed to a Vector3", toParse.c_str()));

	try
	{
		x = std::stof(components[0]);
		y = std::stof(components[1]);
		z = std::stof(components[2]);
	}
	catch (std::exception&)
	{
		ERROR_AND_DIE(Stringf("String %s cannot be parsed to a Vector3", toParse.c_str()));
	}
}


//-----------------------------------------------------------------------------------------------
void Vector3::Normalize()
{
	float length = Length();
	if (length == 0 || length == 1.f)
		return;

	float oneOverLength = 1.f / length;

	x *= oneOverLength;
	y *= oneOverLength;
	z *= oneOverLength;
}


//-----------------------------------------------------------------------------------------------
Vector3 Vector3::Normalized() const
{
	Vector3 result(*this);
	result.Normalize();
	return result;
}


//-----------------------------------------------------------------------------------------------
float Vector3::Length() const
{
	float sqrLength = LengthSquared();
	if (sqrLength == 1.f)
		return 1.f;

	return sqrtf(sqrLength);
}
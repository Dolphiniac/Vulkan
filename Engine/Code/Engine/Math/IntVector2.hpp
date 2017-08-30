#pragma once

#include <string>


//-----------------------------------------------------------------------------------------------
class IntVector3;
class Vector2;
class Vector3;


//-----------------------------------------------------------------------------------------------
class IntVector2
{
public:
	int x;
	int y;

public:
	explicit IntVector2(int newX, int newY);
	IntVector2();
	IntVector2(const IntVector2& toCopy);
	IntVector2(const IntVector3& toCopy);
	IntVector2(const Vector2& toCopy);
	IntVector2(const Vector3& toCopy);
	IntVector2(const std::string& toParse);
	bool operator==(const IntVector2& rightVec) const;
	bool operator!=(const IntVector2& rightVec) const;
	bool operator<(const IntVector2& rightVec) const;
	IntVector2 operator-(const IntVector2& rightVec) const;
	int LengthSquared() const;
	static IntVector2 Zero;
};


//-----------------------------------------------------------------------------------------------
inline IntVector2::IntVector2(int newX, int newY)
: x(newX)
, y(newY)
{
}


//-----------------------------------------------------------------------------------------------
inline IntVector2::IntVector2(const IntVector2& toCopy)
: x(toCopy.x)
, y(toCopy.y)
{
}


//-----------------------------------------------------------------------------------------------
inline IntVector2::IntVector2()
: x(0)
, y(0)
{
}

//-----------------------------------------------------------------------------------------------
inline bool IntVector2::operator==(const IntVector2& rightVec) const
{
	return (x == rightVec.x) && (y == rightVec.y);
}


//-----------------------------------------------------------------------------------------------
inline bool IntVector2::operator!=(const IntVector2& rightVec) const
{
	return !(*this == rightVec);
}


//-----------------------------------------------------------------------------------------------
inline bool IntVector2::operator<(const IntVector2& rightVec) const
{
	if (y < rightVec.y)
	{
		return true;
	}
	else if (y > rightVec.y)
	{
		return false;
	}
	else
	{
		return x < rightVec.x;
	}
}


//-----------------------------------------------------------------------------------------------
inline IntVector2 IntVector2::operator-(const IntVector2& rightVec) const
{
	return IntVector2(x - rightVec.x, y - rightVec.y);
}


//-----------------------------------------------------------------------------------------------
inline int IntVector2::LengthSquared() const
{
	return x * x + y * y;
}
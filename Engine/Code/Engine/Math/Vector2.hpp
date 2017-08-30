#pragma once

#include <string>


//-----------------------------------------------------------------------------------------------
class IntVector2;
class Vector3;
class IntVector3;


//-----------------------------------------------------------------------------------------------
class Vector2
{
public:
	void SetXY(float newX, float newY);
	Vector2 operator+(const Vector2& rightVec) const;
	Vector2 operator-(const Vector2& rightVec) const;
	Vector2 operator*(float scalar) const;
	void operator=(const Vector2& rightVec);
	void operator+=(const Vector2& rightVec);
	void operator-=(const Vector2& rightVec);
	void operator*=(float scalar);
	bool operator==(const Vector2& rightVec) const;
	bool operator!=(const Vector2& rightVect) const;
	Vector2 operator-() const;
	Vector2();
	Vector2(const Vector2& toCopy);
	Vector2(const IntVector2& toCopy);
	Vector2(const Vector3& toCopy);
	Vector2(const IntVector3& toCopy);
	Vector2(const std::string& toParse);
	Vector2(float uniformComponent);
	explicit Vector2(float startX, float startY);
	float LengthSquared();
	float Length();
	void Normalize();
	Vector2 Normalized() const;


	friend Vector2 operator*(float scalar, const Vector2& Vect);
	static float Dot(const Vector2& forward, const Vector2& toPlayer);

public:
	static Vector2 Zero;
	float x;
	float y;
};


//-----------------------------------------------------------------------------------------------
inline Vector2::Vector2()
: x(0.f)
, y(0.f)
{
}


//-----------------------------------------------------------------------------------------------
inline Vector2::Vector2(float startX, float startY)
: x(startX)
, y(startY)
{
}


//-----------------------------------------------------------------------------------------------
inline Vector2::Vector2(const Vector2& toCopy)
: x(toCopy.x)
, y(toCopy.y)
{
}


//-----------------------------------------------------------------------------------------------
inline Vector2::Vector2(float uniformComponent)
: x(uniformComponent)
, y(uniformComponent)
{

}


//-----------------------------------------------------------------------------------------------
inline void Vector2::operator=(const Vector2& toCopy)
{
	this->SetXY(toCopy.x, toCopy.y);
}


//-----------------------------------------------------------------------------------------------
inline void Vector2::SetXY(float newX, float newY)
{
	x = newX;
	y = newY;
}


//-----------------------------------------------------------------------------------------------
inline Vector2 Vector2::operator+(const Vector2& rightVec) const
{
	Vector2 result;
	result.SetXY(this->x + rightVec.x, this->y + rightVec.y);
	return result;
}


//-----------------------------------------------------------------------------------------------
inline Vector2 Vector2::operator-(const Vector2& rightVec) const
{
	Vector2 result;
	result.SetXY(this->x - rightVec.x, this->y - rightVec.y);
	return result;
}


//-----------------------------------------------------------------------------------------------
inline Vector2 Vector2::operator*(float scalar) const
{
	Vector2 result;
	result.SetXY(this->x * scalar, this->y * scalar);
	return result;
}


inline float Vector2::Dot(const Vector2& forward, const Vector2& toPlayer)
{
	return forward.x * toPlayer.x + forward.y * toPlayer.y;
}

//-----------------------------------------------------------------------------------------------
inline void Vector2::operator+=(const Vector2& rightVec)
{
	*this = *this + rightVec;
}


//-----------------------------------------------------------------------------------------------
inline void Vector2::operator-=(const Vector2& rightVec)
{
	*this = *this - rightVec;
}


//-----------------------------------------------------------------------------------------------
inline void Vector2::operator*=(float scalar)
{
	*this = *this * scalar;
}


//-----------------------------------------------------------------------------------------------
inline Vector2 operator*(float scalar, const Vector2& Vect)
{
	Vector2 result;
	result.SetXY(scalar * Vect.x, scalar * Vect.y);
	return result;
}


//-----------------------------------------------------------------------------------------------
inline float Vector2::LengthSquared()
{
	return (x * x + y * y);
}


//-----------------------------------------------------------------------------------------------
inline Vector2 Vector2::operator-() const
{
	return Vector2(-x, -y);
}


//-----------------------------------------------------------------------------------------------
inline bool Vector2::operator==(const Vector2& rightVec) const
{
	return (x == rightVec.x) && (y == rightVec.y);
}


//-----------------------------------------------------------------------------------------------
inline bool Vector2::operator!=(const Vector2& rightVec) const
{
	return !(*this == rightVec);
}
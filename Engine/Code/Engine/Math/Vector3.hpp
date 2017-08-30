#pragma once

#pragma warning (disable: 4201)


//-----------------------------------------------------------------------------------------------
class IntVector3;
class Vector2;
class IntVector2;

#include <string>

//-----------------------------------------------------------------------------------------------
class Vector3
{
public:
	Vector3();
	explicit Vector3(float _x, float _y, float _z);
	Vector3(const std::string& toParse);
	Vector3(const Vector3& toCopy);
	Vector3(const IntVector3& toCopy);
	Vector3(const Vector2& toCopy);
	Vector3(const IntVector2& toCopy);
	void operator=(const Vector3& toCopy);
	Vector3 operator-() const;
	Vector3 operator+(const Vector3& rightVec) const;
	Vector3 operator-(const Vector3& rightVec) const;
	Vector3 operator*(float scalar) const;
	friend Vector3 operator*(float scalar, const Vector3& toScale);
	void operator+=(const Vector3& rightVec);
	void operator-=(const Vector3& rightVec);
	void operator*=(float scalar);
	bool operator==(const Vector3& rightVec) const;
	bool operator!=(const Vector3& rightVec) const;
	void Normalize();
	Vector3 Normalized() const;
	float Length() const;
	float LengthSquared() const;

	static float Dot(const Vector3& leftVec, const Vector3& rightVec);
	static Vector3 Cross(const Vector3& leftVec, const Vector3& rightVec);

public:
	union
	{
		struct 
		{
			float x;
			float y;
			float z;
		};
		struct  
		{
			float pitch;
			float yaw;
			float roll;
		};
	};
	static Vector3 Zero;
	static Vector3 Up;
	static Vector3 Right;
	static Vector3 Forward;
};


//-----------------------------------------------------------------------------------------------
inline Vector3::Vector3()
: Vector3(Vector3::Zero)
{

}


//-----------------------------------------------------------------------------------------------
inline Vector3::Vector3(float _x, float _y, float _z)
: x(_x)
, y(_y)
, z(_z)
{

}


//-----------------------------------------------------------------------------------------------
inline Vector3::Vector3(const Vector3& toCopy)
: x(toCopy.x)
, y(toCopy.y)
, z(toCopy.z)
{

}


//-----------------------------------------------------------------------------------------------
inline void Vector3::operator=(const Vector3& toCopy)
{
	x = toCopy.x;
	y = toCopy.y;
	z = toCopy.z;
}


//-----------------------------------------------------------------------------------------------
inline Vector3 Vector3::operator-() const
{
	return Vector3(-x, -y, -z);
}


//-----------------------------------------------------------------------------------------------
inline Vector3 Vector3::operator+(const Vector3& rightVec) const
{
	return Vector3(x + rightVec.x, y + rightVec.y, z + rightVec.z);
}


//-----------------------------------------------------------------------------------------------
inline Vector3 Vector3::operator-(const Vector3& rightVec) const
{
	return *this + (-rightVec);
}


//-----------------------------------------------------------------------------------------------
inline Vector3 Vector3::operator*(float scalar) const
{
	return Vector3(scalar * x, scalar * y, scalar * z);
}


//-----------------------------------------------------------------------------------------------
inline Vector3 operator*(float scalar, const Vector3& toScale)
{
	return toScale * scalar;
}


//-----------------------------------------------------------------------------------------------
inline void Vector3::operator+=(const Vector3& rightVec)
{
	*this = *this + rightVec;
}


//-----------------------------------------------------------------------------------------------
inline void Vector3::operator-=(const Vector3& rightVec)
{
	*this = *this - rightVec;
}


//-----------------------------------------------------------------------------------------------
inline void Vector3::operator*=(float scalar)
{
	*this = *this * scalar;
}


//-----------------------------------------------------------------------------------------------
inline float Vector3::LengthSquared() const
{
	return x * x + y * y + z * z;
}


//-----------------------------------------------------------------------------------------------
inline float Vector3::Dot(const Vector3& leftVec, const Vector3& rightVec)
{
	return (leftVec.x * rightVec.x) + (leftVec.y * rightVec.y) + (leftVec.z * rightVec.z);
}


//-----------------------------------------------------------------------------------------------
inline Vector3 Vector3::Cross(const Vector3& leftVec, const Vector3& rightVec)
{
	float x = (leftVec.y * rightVec.z) - (rightVec.y * leftVec.z);
	float y = -(leftVec.x * rightVec.z) + (rightVec.x * leftVec.z);
	float z = (leftVec.x * rightVec.y) - (rightVec.x * leftVec.y);

	return Vector3(x, y, z);
}


//-----------------------------------------------------------------------------------------------
inline bool Vector3::operator==(const Vector3& rightVec) const
{
	return x == rightVec.x && y == rightVec.y && z == rightVec.z;
}


//-----------------------------------------------------------------------------------------------
inline bool Vector3::operator!=(const Vector3& rightVec) const
{
	return !(*this == rightVec);
}
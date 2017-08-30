#pragma once


//-----------------------------------------------------------------------------------------------
class Vector2;
class Vector3;
class IntVector2;


//-----------------------------------------------------------------------------------------------
class IntVector3
{
public:
	IntVector3();
	explicit IntVector3(int _x, int _y, int _z);
	IntVector3(const IntVector3& toCopy);
	IntVector3(const Vector3& toCopy);
	IntVector3(const Vector2& toCopy);
	IntVector3(const IntVector2& toCopy);
	void operator=(const IntVector3& toCopy);
	IntVector3 operator-() const;
	IntVector3 operator+(const IntVector3& rightVec) const;
	IntVector3 operator-(const IntVector3& rightVec) const;
	IntVector3 operator*(int scalar) const;
	friend IntVector3 operator*(int scalar, const IntVector3& toScale);
	void operator+=(const IntVector3& rightVec);
	void operator-=(const IntVector3& rightVec);
	void operator*=(int scalar);
	float Length() const;
	int LengthSquared() const;

	static IntVector3 Zero;
	static int Dot(const IntVector3& leftVec, const IntVector3& rightVec);

public:
	int x;
	int y;
	int z;
};


//-----------------------------------------------------------------------------------------------
inline IntVector3::IntVector3()
: IntVector3(IntVector3::Zero)
{

}


//-----------------------------------------------------------------------------------------------
inline IntVector3::IntVector3(int _x, int _y, int _z)
: x(_x)
, y(_y)
, z(_z)
{

}


//-----------------------------------------------------------------------------------------------
inline IntVector3::IntVector3(const IntVector3& toCopy)
: x(toCopy.x)
, y(toCopy.y)
, z(toCopy.z)
{

}


//-----------------------------------------------------------------------------------------------
inline void IntVector3::operator=(const IntVector3& toCopy)
{
	x = toCopy.x;
	y = toCopy.y;
	z = toCopy.z;
}


//-----------------------------------------------------------------------------------------------
inline IntVector3 IntVector3::operator-() const
{
	return IntVector3(-x, -y, -z);
}


//-----------------------------------------------------------------------------------------------
inline IntVector3 IntVector3::operator+(const IntVector3& rightVec) const
{
	return IntVector3(x + rightVec.x, y + rightVec.y, z + rightVec.z);
}


//-----------------------------------------------------------------------------------------------
inline IntVector3 IntVector3::operator-(const IntVector3& rightVec) const
{
	return *this + (-rightVec);
}


//-----------------------------------------------------------------------------------------------
inline IntVector3 IntVector3::operator*(int scalar) const
{
	return IntVector3(scalar * x, scalar * y, scalar * z);
}


//-----------------------------------------------------------------------------------------------
inline IntVector3 operator*(int scalar, const IntVector3& toScale)
{
	return toScale * scalar;
}


//-----------------------------------------------------------------------------------------------
inline void IntVector3::operator+=(const IntVector3& rightVec)
{
	*this = *this + rightVec;
}


//-----------------------------------------------------------------------------------------------
inline void IntVector3::operator-=(const IntVector3& rightVec)
{
	*this = *this - rightVec;
}


//-----------------------------------------------------------------------------------------------
inline void IntVector3::operator*=(int scalar)
{
	*this = *this * scalar;
}


//-----------------------------------------------------------------------------------------------
inline int IntVector3::LengthSquared() const
{
	return x * x + y * y + z * z;
}


//-----------------------------------------------------------------------------------------------
inline int IntVector3::Dot(const IntVector3& leftVec, const IntVector3& rightVec)
{
	return (leftVec.x * rightVec.x) + (leftVec.y * rightVec.y) + (leftVec.z * rightVec.z);
}
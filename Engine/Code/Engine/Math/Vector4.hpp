#pragma once

#pragma warning(disable:4201)


//-----------------------------------------------------------------------------------------------
__declspec(align(16)) class Vector4
{
public:
	union
	{
		struct
		{
			float x, y, z, w;
		};
		struct  
		{
			float r, g, b, a;
		};
	};

	inline Vector4();
	inline Vector4(const Vector4& rightVec);
	Vector4(float _x, float _y, float _z, float _w) : w(_w), x(_x), y(_y), z(_z) {}
	Vector4 operator*(const class Matrix44& mat) const;
	void operator=(const Vector4& rightVec);
	Vector4 operator-(const Vector4& rightVec);
	void operator*=(const Matrix44& mat);
	void operator*=(float scalar);
	class Vector3 XYZ() const;

	static Vector4 Zero;
	static float Dot(const Vector4& leftVec, const Vector4& rightVec);
};


//-----------------------------------------------------------------------------------------------
class IntVector4
{
public:
	union
	{
		struct
		{
			int x, y, z, w;
		};
		struct
		{
			int r, g, b, a;
		};
	};

};


//-----------------------------------------------------------------------------------------------
const Vector4 V_WHITE(1.f, 1.f, 1.f, 1.f);
const Vector4 V_BLACK(0.f, 0.f, 0.f, 1.f);
const Vector4 V_RED(1.f, 0.f, 0.f, 1.f);
const Vector4 V_GREEN(0.f, 1.f, 0.f, 1.f);
const Vector4 V_BLUE(0.f, 0.f, 1.f, 1.f);
const Vector4 V_GREY(.5f, .5f, .5f, 1.f);
const Vector4 V_YELLOW(1.f, 1.f, 0.f, 1.f);
const Vector4 V_MAGENTA(1.f, 0.f, 1.f, 1.f);
const Vector4 V_ORANGE(1.f, .647f, 0.f, 1.f);


//-----------------------------------------------------------------------------------------------
Vector4::Vector4()
{
	*this = Zero;
}


//-----------------------------------------------------------------------------------------------
Vector4::Vector4(const Vector4& rightVec)
	: w(rightVec.w)
	, x(rightVec.x)
	, y(rightVec.y)
	, z(rightVec.z)
{
}
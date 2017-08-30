#pragma once


//-----------------------------------------------------------------------------------------------
typedef class QuCVector4 byte4;


//-----------------------------------------------------------------------------------------------
class QuCVector4
{
public:
	QuCVector4() : r(0), g(0), b(0), a(0) {}
	QuCVector4(byte _r, byte _g, byte _b, byte _a) : r(_r), g(_g), b(_b), a(_a) {}
	QuCVector4(byte _all) : r(_all), g(_all), b(_all), a(_all) {}

public:
	class QuCVector2 XY() const;
	class QuCVector3 XYZ() const;

public:
	byte r;
	byte g;
	byte b;
	byte a;
};


//-----------------------------------------------------------------------------------------------
typedef class QuVector4 float4;


//-----------------------------------------------------------------------------------------------
class QuVector4
{
public:
	QuVector4() : x(0.f), y(0.f), z(0.f), w(0.f) {}
	QuVector4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
	QuVector4(float _all) : x(_all), y(_all), z(_all), w(_all) {}

public:
	class QuVector2 XY() const;
	class QuVector3 XYZ() const;

public:
	union
	{
		float x;
		float r;
	};
	union
	{
		float y;
		float g;
	};
	union
	{
		float z;
		float b;
	};
	union
	{
		float w;
		float a;
	};

public:
	static const QuVector4 Zero;
};
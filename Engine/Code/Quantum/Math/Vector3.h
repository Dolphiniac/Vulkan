#pragma once


//-----------------------------------------------------------------------------------------------
typedef class QuCVector3 byte3;


//-----------------------------------------------------------------------------------------------
class QuCVector3
{
public:
	QuCVector3() : r(0), g(0), b(0) {}
	QuCVector3(byte _r, byte _g, byte _b) : r(_r), g(_g), b(_b) {}
	QuCVector3(byte _all) : r(_all), g(_all), b(_all) {}
	QuCVector3(const class QuCVector2& _rg, byte _b);
	QuCVector3(byte _r, const class QuCVector2& _gb);

public:
	class QuCVector2 XY() const;

public:
	byte r;
	byte g;
	byte b;
};


//-----------------------------------------------------------------------------------------------
typedef class QuVector3 float3;


//-----------------------------------------------------------------------------------------------
class QuVector3
{
public:
	QuVector3() : x(0.f), y(0.f), z(0.f) {}
	QuVector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

public:
	class QuVector2 XY() const;

public:
	union
	{
		float x;
		float r;
		float u;
	};
	union
	{
		float y;
		float g;
		float v;
	};
	union
	{
		float z;
		float b;
		float w;
	};

public:
	static const QuVector3 Zero;
};
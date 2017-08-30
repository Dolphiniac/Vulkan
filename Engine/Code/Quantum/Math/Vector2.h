#pragma once


//-----------------------------------------------------------------------------------------------
typedef class QuCVector2 byte2;


//-----------------------------------------------------------------------------------------------
class QuCVector2
{
public:
	QuCVector2() : r(0), g(0) {}
	QuCVector2(byte _r, byte _g) : r(_r), g(_g) {}
	QuCVector2(byte _all) : r(_all), g(_all) {}

public:
	byte r;
	byte g;
};


//-----------------------------------------------------------------------------------------------
typedef class QuVector2 float2;


class QuVector2
{
public:
	QuVector2() : x(0.f), y(0.f) {}
	QuVector2(float _x, float _y) : x(_x), y(_y) {}

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

public:
	static const QuVector2 Zero;
};
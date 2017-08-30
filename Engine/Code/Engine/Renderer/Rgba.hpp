#pragma once

#include "Engine/Math/Vector4.hpp"
#include "Quantum/Renderer/Color.h"


//-----------------------------------------------------------------------------------------------
struct Rgba
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
	Rgba(float _r, float _g, float _b, float _a)
		: r((unsigned char)(_r * 255.f))
		, g((unsigned char)(_g * 255.f))
		, b((unsigned char)(_b * 255.f))
		, a((unsigned char)(_a * 255.f))
	{

	}
	Rgba()
		: r(255)
		, g(255)
		, b(255)
		, a(255)
	{}
	Rgba(const QuColor& color) : r(color.r), g(color.g), b(color.b), a(color.a) {}
};


//-----------------------------------------------------------------------------------------------
const Rgba WHITE = { 1.f, 1.f, 1.f, 1.f };
const Rgba BLACK = { 0.f, 0.f, 0.f, 1.f };
const Rgba RED = { 1.f, 0.f, 0.f, 1.f };
const Rgba GREEN = { 0.f, 1.f, 0.f, 1.f };
const Rgba BLUE = { 0.f, 0.f, 1.f, 1.f };
const Rgba GREY = { .5f, .5f, .5f, 1.f };
const Rgba YELLOW = { 1.f, 1.f, 0.f, 1.f };
const Rgba MAGENTA = { 1.f, 0.f, 1.f, 1.f };
const Rgba ORANGE = { 1.f, .647f, 0.f, 1.f };


//-----------------------------------------------------------------------------------------------
inline Vector4 ToVec4(const Rgba& color)
{
	return Vector4((float)color.r / 255.f, (float)color.g / 255.f, (float)color.b / 255.f, (float)color.a / 255.f);
}


//-----------------------------------------------------------------------------------------------
inline Rgba GetComplementaryColor(const Rgba& color)
{
	unsigned char highestColor = color.r;
	int highestIndex = 0;
	Rgba result;
	result.a = color.a;
	if (color.g > highestColor)
	{
		highestColor = color.g;
		highestIndex = 1;
	}
	if (color.b > highestColor)
	{
		highestColor = color.b;
		highestIndex = 2;
	}

	switch (highestIndex)
	{
	case 0:
		result.r = color.r;
		result.g = 255 - color.g;
		result.b = 255 - color.b;
		break;
	case 1:
		result.r = 255 - color.r;
		result.g = color.g;
		result.b = 255 - color.b;
		break;
	case 2:
		result.r = 255 - color.r;
		result.g = 255 - color.g;
		result.b = color.b;
		break;
	}

	return result;
}
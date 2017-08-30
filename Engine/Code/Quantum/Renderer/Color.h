#pragma once


//-----------------------------------------------------------------------------------------------
struct QuColor
{
public:
	QuColor() = default;
	QuColor(uint8 _r, uint8 _g, uint8 _b, uint8 _a) : r(_r), b(_b), g(_g), a(_a) {}

public:
	uint8 r, g, b, a;

public:
	static const QuColor WHITE;
	static const QuColor BLACK;
	static const QuColor BAD_COLOR;
};
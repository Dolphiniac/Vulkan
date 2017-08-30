#pragma once

#include "Engine/Renderer/RGBA.hpp"


//-----------------------------------------------------------------------------------------------
struct TextEffect
{
	float wave = 1.f;
	bool shake = false;
	float dilate = 0.f;
	bool pop = false;
	Rgba color1 = WHITE;
	Rgba color2 = WHITE;
	bool rainbow = false;
};
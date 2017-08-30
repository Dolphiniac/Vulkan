#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"


//-----------------------------------------------------------------------------------------------
class Light
{
public:
	Vector3 m_position;
	Vector4 m_colorAndBrightness;
	float m_innerRadius;
	float m_outerRadius;
	Vector3 m_forward;
	float m_innerAperture;
	float m_outerAperture;
	bool m_isDirectional;

public:
	Light();
	Light(const Vector4& colorAndBrightness, float innerRadius = 100.f, float outerRadius = 100.f, const Vector3& forwardVec = Vector3::Zero, 
		float innerAperture = 0.f, float outerAperture = 0.f, bool isDirectional = false)
		: m_position(Vector3::Zero), m_colorAndBrightness(colorAndBrightness),
		m_innerRadius(innerRadius), m_outerRadius(outerRadius),
		m_forward(forwardVec), m_innerAperture(innerAperture), m_outerAperture(outerAperture),
		m_isDirectional(isDirectional) {}

	static Vector3* MakePosArray(const Light* lights, int numLights);
	static Vector3* MakeColorArray(const Light* lights, int numLights);
	static float* MakeBrightnessArray(const Light* lights, int numLights);
	static float* MakeInnerRadiusArray(const Light* lights, int numLights);
	static float* MakeOuterRadiusArray(const Light* lights, int numLights);
	static Vector3* MakeForwardArray(const Light* lights, int numLights);
	static float* MakeInnerApertureArray(const Light* lights, int numLights);
	static float* MakeOuterApertureArray(const Light* lights, int numLights);
	static float* MakeHasForwardArray(const Light* lights, int numLights);
};

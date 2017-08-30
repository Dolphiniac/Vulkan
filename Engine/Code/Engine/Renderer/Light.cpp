#include "Engine/Renderer/Light.hpp"


//-----------------------------------------------------------------------------------------------
Light::Light()
	: m_position(Vector3::Zero)
	, m_colorAndBrightness(Vector4::Zero)
	, m_outerRadius(100.f)
	, m_innerRadius(100.f)
	, m_forward(Vector3::Zero)
	, m_innerAperture(0.f)
	, m_outerAperture(0.f)
	, m_isDirectional(false)
{

}


//-----------------------------------------------------------------------------------------------
Vector3* Light::MakePosArray(const Light* lights, int numLights)
{
	Vector3* result = new Vector3[numLights];
	for (int i = 0; i < numLights; i++)
	{
		result[i] = lights[i].m_position;
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
Vector3* Light::MakeColorArray(const Light* lights, int numLights)
{
	Vector3* result = new Vector3[numLights];
	for (int i = 0; i < numLights; i++)
	{
		result[i] = lights[i].m_colorAndBrightness.XYZ();
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
float* Light::MakeBrightnessArray(const Light* lights, int numLights)
{
	float* result = new float[numLights];
	for (int i = 0; i < numLights; i++)
	{
		result[i] = lights[i].m_colorAndBrightness.a;
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
float* Light::MakeInnerRadiusArray(const Light* lights, int numLights)
{
	float* result = new float[numLights];
	for (int i = 0; i < numLights; i++)
	{
		result[i] = lights[i].m_innerRadius;
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
float* Light::MakeOuterRadiusArray(const Light* lights, int numLights)
{
	float* result = new float[numLights];
	for (int i = 0; i < numLights; i++)
	{
		result[i] = lights[i].m_outerRadius;
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
Vector3* Light::MakeForwardArray(const Light* lights, int numLights)
{
	Vector3* result = new Vector3[numLights];
	for (int i = 0; i < numLights; i++)
	{
		result[i] = lights[i].m_forward;
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
float* Light::MakeInnerApertureArray(const Light* lights, int numLights)
{
	float* result = new float[numLights];
	for (int i = 0; i < numLights; i++)
	{
		result[i] = lights[i].m_innerAperture;
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
float* Light::MakeOuterApertureArray(const Light* lights, int numLights)
{
	float* result = new float[numLights];
	for (int i = 0; i < numLights; i++)
	{
		result[i] = lights[i].m_outerAperture;
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
float* Light::MakeHasForwardArray(const Light* lights, int numLights)
{
	float* result = new float[numLights];
	for (int i = 0; i < numLights; i++)
	{
		if (lights[i].m_isDirectional)
		{
			result[i] = 1.f;
		}
		else
		{
			result[i] = 0.f;
		}
	}

	return result;
}
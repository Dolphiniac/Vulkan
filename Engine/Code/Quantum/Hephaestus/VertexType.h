#pragma once

#include "Quantum/Hephaestus/Declarations.h"

#include "Quantum/Math/MathCommon.h"


//-----------------------------------------------------------------------------------------------
struct HVertexP
{
	float3 position;
};


//-----------------------------------------------------------------------------------------------
struct HVertexPCT
{
	float3 position;
	float4 color;
	float2 uv;
};


//-----------------------------------------------------------------------------------------------
struct HVertexPCTN
{
	float3 position;
	float4 color;
	float2 uv;
	float3 normal;
};


//-----------------------------------------------------------------------------------------------
struct HVertexPCTNT
{
	float3 position;
	float4 color;
	float2 uv;
	float4 normal;
	float3 tangent;
};


//-----------------------------------------------------------------------------------------------
class HVertexType
{
	friend class HPipelineGenerator;
	friend class HManager;

private:
	void Initialize(EVertexType thisType);
	void Deinitialize();

private:
	static void InitializeTypes();
	static void DeinitializeTypes();

private:
	static HVertexType s_vertexTypes[H_VERTEX_TYPE_COUNT];

private:
	HephPipelineVertexInputStateCreateInfo m_inputState = nullptr;
	HephVertexInputBindingDescription m_bindingDesc = nullptr;
	HephVertexInputAttributeDescription m_attribDesc = nullptr;
};
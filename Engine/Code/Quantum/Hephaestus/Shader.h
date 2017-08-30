#pragma once

#include "Quantum/Hephaestus/Declarations.h"

#include <vector>


//-----------------------------------------------------------------------------------------------
class HShader
{
	friend class HManager;
	friend class HPipelineGenerator;

public:
	HShader(const char* pathOrCode, HShaderTypeBits shaderType, bool fromFile = true, bool spirv = false);
	~HShader();
	HephPipelineShaderStageCreateInfo m_createInfo = nullptr;

	std::vector<uint32> m_spirvCode;
private:
	HephShaderModule m_module = H_NULL_HANDLE;

private:
	static void Deinitialize();
};
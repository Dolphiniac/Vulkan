#include "Quantum/Hephaestus/Shader.h"
#include "Quantum/Hephaestus/Manager.h"
#include "Quantum/Hephaestus/LogicalDevice.h"
#include "Quantum/Hephaestus/BufferDescriptor.h"
#include "Engine/Core/FileUtils.hpp"
#include "Quantum/Hephaestus/Spirv.h"

#include "ThirdParty/shaderc/shaderc.hpp"

#include <vulkan.h>

#pragma comment (lib, "shaderc_combined.lib")


//-----------------------------------------------------------------------------------------------
static shaderc::Compiler* s_compiler = nullptr;


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HShader::HShader(const char* pathOrCode, HShaderTypeBits shaderType, bool fromFile /* = true */, bool spirv /* = false */)
{
	//Initialize the compiler if this is our first shader
	if (!s_compiler)
	{
		s_compiler = new shaderc::Compiler();
	}

	shaderc_shader_kind kind = shaderc_glsl_infer_from_source;

	VkShaderStageFlagBits stage;
	
	//Two different APIs need the stage, so we'll set those based on our own enum
	switch (shaderType)
	{
	case H_SHADER_TYPE_VERTEX_BIT:
		kind = shaderc_glsl_vertex_shader;
		stage = VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case H_SHADER_TYPE_FRAGMENT_BIT:
		kind = shaderc_glsl_fragment_shader;
		stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	case H_SHADER_TYPE_TESSELLATION_CONTROL_BIT:
		kind = shaderc_glsl_tess_control_shader;
		stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		break;
	case H_SHADER_TYPE_TESSELLATION_EVALUATION_BIT:
		kind = shaderc_glsl_tess_evaluation_shader;
		stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		break;
	case H_SHADER_TYPE_GEOMETRY_BIT:
		kind = shaderc_glsl_geometry_shader;
		stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		break;
	default:
		break;
	}
	QuString inputGlsl = *HGlobalShaderIncludeBuilder::s_globalShader;
	std::vector<char> fileGlsl;
	
	if (fromFile)
	{
		//We passed the path to a shader file, so we'll load it in and add a null terminator
		LoadBinaryFileToBuffer(pathOrCode, fileGlsl);
		fileGlsl.push_back('\0');
		inputGlsl += fileGlsl.data();
	}
	else
	{
		//We passed the code itself, so just put it in raw
		inputGlsl += pathOrCode;
	}
	HLogicalDevice* device = HManager::GetLogicalDevice();

	if (!spirv)
	{
		shaderc::SpvCompilationResult result = s_compiler->CompileGlslToSpv(inputGlsl.GetRaw(), kind, "unused");
		ASSERT_OR_DIE(result.GetNumErrors() == 0, result.GetErrorMessage()); //Ensure no errors occurred during shader compile. Fatal


		//The compilation result is in 32-bit words, and we need bytes
		m_spirvCode.resize(result.end() - result.begin());
		memcpy(&m_spirvCode[0], result.begin(), m_spirvCode.size() * 4);
	}
	else
	{
		m_spirvCode.resize((fileGlsl.size() - 1) / 4);
		memcpy(&m_spirvCode[0], fileGlsl.data(), m_spirvCode.size() * 4);
	}

	//Since we want to enforce that an HShader is of a particular stage, and specialization info is really hairy,
	//we'll go ahead and create a shader stage info for it that can be easily plugged into a pipeline
	m_createInfo = new VkPipelineShaderStageCreateInfo();
	m_createInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	m_createInfo->pNext = nullptr;
	m_createInfo->flags = 0;
	m_createInfo->module = H_NULL_HANDLE;
	m_createInfo->pSpecializationInfo = nullptr;
	m_createInfo->pName = "main";
	m_createInfo->stage = stage;
}


//-----------------------------------------------------------------------------------------------
HShader::~HShader()
{
	SAFE_DELETE(m_createInfo);

	HLogicalDevice* device = HManager::GetLogicalDevice();

	vkDestroyShaderModule(*device, m_module, nullptr);
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
STATIC void HShader::Deinitialize()
{
	SAFE_DELETE(s_compiler);
}
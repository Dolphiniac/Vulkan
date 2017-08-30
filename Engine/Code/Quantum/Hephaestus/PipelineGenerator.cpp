#include "Quantum/Hephaestus/PipelineGenerator.h"
#include "Quantum/Hephaestus/Manager.h"
#include "Quantum/Hephaestus/LogicalDevice.h"
#include "Quantum/Hephaestus/VertexType.h"
#include "Quantum/Hephaestus/Shader.h"
#include "Quantum/Hephaestus/BufferDescriptor.h"
#include "Quantum/Hephaestus/Pipeline.h"
#include "Quantum/Hephaestus/RenderPass.h"
#include "Quantum/Hephaestus/Spirv.h"
#include "Quantum/Core/String.h"
#include "Engine/Core/XMLUtils.hpp"

#include <vulkan.h>


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HPipelineGenerator::HPipelineGenerator()
{
	VkPipelineCacheCreateInfo cacheCreateInfo;
	cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	cacheCreateInfo.pNext = nullptr;
	cacheCreateInfo.flags = 0;
	cacheCreateInfo.initialDataSize = 0;
	cacheCreateInfo.pInitialData = nullptr;
	HLogicalDevice* device = HManager::GetLogicalDevice();

	H_ASSERT(vkCreatePipelineCache(*device, &cacheCreateInfo, nullptr, &m_cache), "Could not create pipeline cache\n");

	Reset();
}


//-----------------------------------------------------------------------------------------------
HPipelineGenerator::~HPipelineGenerator()
{
	HLogicalDevice* device = HManager::GetLogicalDevice();

	vkDestroyPipelineCache(*device, m_cache, nullptr);
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HPipeline* HPipelineGenerator::GenerateGraphicsPipeline()
{
	HLogicalDevice* device = HManager::GetLogicalDevice();
	HReflectionData pipelineReflection;

	std::vector<HephPipelineShaderStageCreateInfo_T> createInfos;
	uint32 numShaders = m_graphicsShaders.size();
	for (uint32 shaderIndex = 0; shaderIndex < numShaders; shaderIndex++)
	{
		HShader* shader = m_graphicsShaders[shaderIndex];
		std::vector<uint32> workingCode = shader->m_spirvCode;
		HSpirv::ReflectUniforms(workingCode, pipelineReflection);

		createInfos.push_back(*shader->m_createInfo);
		HephPipelineShaderStageCreateInfo createInfo = &createInfos.back();
		
		VkShaderModuleCreateInfo shaderModuleCreateInfo;
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.pNext = nullptr;
		shaderModuleCreateInfo.flags = 0;
		shaderModuleCreateInfo.codeSize = workingCode.size() * 4;
		shaderModuleCreateInfo.pCode = &workingCode[0];

		H_ASSERT(vkCreateShaderModule(*device, &shaderModuleCreateInfo, nullptr, &createInfo->module), "Could not create shader module\n");

	}

	m_graphicsPipelineInfo->stageCount = createInfos.size();
	m_graphicsPipelineInfo->pStages = &createInfos[0];

	HPipeline* result = new HPipeline(m_graphicsPipelineInfo, m_cache, pipelineReflection);
	return result;
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::Reset()
{
	ReleaseMemory();

	InitDefaultGraphicsPipeline();

	InitDefaultComputePipeline();
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::AddShader(HShader* program, bool isCompute /* = false */)
{
	if (isCompute)
	{
		m_computePipelineInfo->stage = *program->m_createInfo;
	}
	else
	{
		m_graphicsShaders.push_back(program);
		m_graphicsPipelineInfo->stageCount = m_graphicsShaders.size();
	}
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::SetVertexType(EVertexType vertexType)
{
	HVertexType& vertexTypeContainer = HVertexType::s_vertexTypes[vertexType];
	
	m_graphicsPipelineInfo->pVertexInputState = vertexTypeContainer.m_inputState;
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::AssignSubpass(HSubpass* subpass)
{
	m_graphicsPipelineInfo->renderPass = subpass->renderPass;
	m_graphicsPipelineInfo->subpass = subpass->subpassIndex;
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::InitializeFromXML(const QuString& pipelineName)
{
	uint32 lastShaderLevelWritten = H_INVALID;
	InitializeFromXML(pipelineName, 0, lastShaderLevelWritten);
}


//-----------------------------------------------------------------------------------------------
HPipeline* HPipelineGenerator::CreateOrGetPipeline(const QuString& name, uint32 renderPassName, uint32 subpassIndex)
{
	const char* mangleString = "yyxz";
	QuString fullName = QuString::F("%s%s%u%s%u", name.GetRaw(), mangleString, renderPassName, mangleString, subpassIndex);

	auto pipelineIter = m_pipelines.find(fullName);
	if (pipelineIter != m_pipelines.end())
	{
		return pipelineIter->second;
	}

	InitializeFromXML(name);
	HSubpass subpass = HManager::GetRenderPassByName(renderPassName)->RetrieveSubpass(subpassIndex);
	AssignSubpass(&subpass);
	HPipeline* result = GenerateGraphicsPipeline();

	m_pipelines.insert(std::make_pair(fullName, result));

	return result;
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::InitializeFromXML(const QuString& pipelineName, uint32 level, uint32& lastShaderLevelWritten)
{
	ReleaseMemory();

	XMLNode rootNode = XMLUtils::GetRootNode(QuString::F("Data/Pipelines/%s.Pipeline.xml", pipelineName.GetRaw()));

	QuString parent = XMLUtils::GetAttribute(rootNode, "parent");
	if (parent != "none")
	{
		//Pre-recurse upward through parent tree so more specific settings override less specific ones
		InitializeFromXML(parent, level + 1, lastShaderLevelWritten);
	}

	QuString rootName = rootNode.getName();
	if (rootName == "GraphicsPipeline")
	{
		InitializeGraphicsPipelineFromXML(rootNode, level, lastShaderLevelWritten);
	}
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::InitializeGraphicsPipelineFromXML(const XMLNode &rootNode, uint32 level, uint32& lastShaderLevelWritten)
{
	if (!m_graphicsPipelineInfo)
	{
		m_graphicsPipelineInfo = new VkGraphicsPipelineCreateInfo();
		m_graphicsPipelineInfo->sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		m_graphicsPipelineInfo->pNext = nullptr;
		m_graphicsPipelineInfo->flags = 0;	//May still want to populate, but maybe from XML?
		m_graphicsPipelineInfo->basePipelineHandle = H_NULL_HANDLE;
		m_graphicsPipelineInfo->basePipelineIndex = H_INVALID_INDEX;
	}

	uint32 numChildren = rootNode.nChildNode();

	for (uint32 childIndex = 0; childIndex < numChildren; childIndex++)
	{
		XMLNode childNode = rootNode.getChildNode(childIndex);
		QuString nodeName = childNode.getName();

		if (nodeName == "Shader")
		{
			ParseShaderNode(childNode, level, lastShaderLevelWritten);
		}
		else if (nodeName == "VertexState")
		{
			ParseVertexStateNode(childNode);
		}
		else if (nodeName == "DynamicState")
		{
			ParseDynamicStateNode(childNode);
		}
		else if (nodeName == "MultisampleState")
		{
			ParseMultisampleStateNode(childNode);
		}
		else if (nodeName == "ColorBlendState")
		{
			ParseColorBlendStateNode(childNode);
		}
		else if (nodeName == "DepthStencilState")
		{
			ParseDepthStencilStateNode(childNode);
		}
		else if (nodeName == "TessellationState")
		{
			ParseTessellationStateNode(childNode);
		}
		else if (nodeName == "ViewportState")
		{
			ParseViewportStateNode(childNode);
		}
		else if (nodeName == "RasterizationState")
		{
			ParseRasterizationStateNode(childNode);
		}
	}
}


//-----------------------------------------------------------------------------------------------
static HShaderTypeBits DeduceFromExtension(const QuString& shaderName, bool* pOutIsSpirv, bool failOnBadDeduction)
{
	std::vector<QuString> tokens = shaderName.SplitOnDelimiter('.');
	QuString lastToken = tokens.back();
	QuString extensionToken = lastToken;
	if (lastToken == "spv")
	{
		extensionToken = tokens[tokens.size() - 2];
		if (pOutIsSpirv)
		{
			*pOutIsSpirv = true;
		}
	}
	else
	{
		if (pOutIsSpirv)
		{
			*pOutIsSpirv = false;
		}
	}

	if (extensionToken == "vert")
	{
		return H_SHADER_TYPE_VERTEX_BIT;
	}
	else if (extensionToken == "frag")
	{
		return H_SHADER_TYPE_FRAGMENT_BIT;
	}
	else
	{
		if (failOnBadDeduction)
		{
			ERROR_AND_DIE("Shader stage could not be deduced\n");
		}
	}
}


#pragma warning (disable: 4701) //
//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseShaderNode(const XMLNode& node, uint32 level, uint32& lastShaderLevelWritten)
{
	if (level != lastShaderLevelWritten)
	{
		m_graphicsShaders.clear();
		lastShaderLevelWritten = level;
	}
	XML_EXTRACT_ATTRIBUTE(node, format);
	XML_EXTRACT_ATTRIBUTE(node, stage);
	XML_EXTRACT_ATTRIBUTE(node, name);
	bool isSpirv;
	HShaderTypeBits shaderStage = DeduceFromExtension(name, &isSpirv, false);
	XML_SET_VAR_THREE_OPTIONS_WITH_ERROR(stage, shaderStage, 
		"", DeduceFromExtension(name, &isSpirv, true),
		"vertex", H_SHADER_TYPE_VERTEX_BIT,
		"fragment", H_SHADER_TYPE_FRAGMENT_BIT,
		"Shader stage qualifier not recognized\n");
	
	QuString filepath = QuString::F("Data/Shaders/%s", name.GetRaw());
	HShader* toAdd = nullptr;

	XML_SET_VAR_THREE_OPTIONS_WITH_ERROR(format, toAdd,
		"", new HShader(filepath.GetRaw(), shaderStage, true, isSpirv),
		"glsl", new HShader(filepath.GetRaw(), shaderStage, true, false),
		"spirv", new HShader(filepath.GetRaw(), shaderStage, true, true),
		"Shader format MUST be \"glsl\" or \"spirv\"\n");

	AddShader(toAdd);

	//m_graphicsShaders.push_back(toAdd);
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseVertexStateNode(const XMLNode& node)
{
	uint32 numChildren = node.nChildNode();

	for (uint32 childIndex = 0; childIndex < numChildren; childIndex++)
	{
		XMLNode childNode = node.getChildNode(childIndex);
		QuString nodeName = childNode.getName();
		if (nodeName == "VertexType")
		{
			ParseVertexTypeNode(childNode);
		}
		else if (nodeName == "InputAssembly")
		{
			ParseInputAssemblyNode(childNode);
		}
		else
		{
			ERROR_AND_DIE("Unrecognized node name for VertexState\n");
		}
	}
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseVertexTypeNode(const XMLNode& node)
{
	XML_EXTRACT_ATTRIBUTE(node, name);
	if (name == "P")
	{
		SetVertexType(H_VERTEX_TYPE_P);
	}
	else if (name == "PCT")
	{
		SetVertexType(H_VERTEX_TYPE_PCT);
	}
	else if (name == "PCTN")
	{
		SetVertexType(H_VERTEX_TYPE_PCTN);
	}
	else if (name == "PCTNT")
	{
		SetVertexType(H_VERTEX_TYPE_PCTNT);
	}
	else
	{
		ERROR_AND_DIE("Vertex type not supported\n");
	}
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseInputAssemblyNode(const XMLNode& node)
{
	if (!m_inputAssemblyCreateInfo)
	{
		m_inputAssemblyCreateInfo = new VkPipelineInputAssemblyStateCreateInfo();
		m_inputAssemblyCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		m_inputAssemblyCreateInfo->pNext = nullptr;
		m_inputAssemblyCreateInfo->flags = 0;

		m_graphicsPipelineInfo->pInputAssemblyState = m_inputAssemblyCreateInfo;
	}

	XML_EXTRACT_ATTRIBUTE(node, topology);
	XML_SET_VAR_ONE_OPTION_ERROR_AND_IGNORE(topology, m_inputAssemblyCreateInfo->topology,
		"triangleList", VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		"Unrecognized topology for input assembly node\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, primitiveRestartEnable);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(primitiveRestartEnable, m_inputAssemblyCreateInfo->primitiveRestartEnable,
		"true", VK_TRUE,
		"false", VK_FALSE,
		"Unrecognized value for primitiveRestartEnable\n",
		"");
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseDynamicStateNode(const XMLNode& node)
{
	if (!m_dynamicCreateInfo)
	{
		m_dynamicCreateInfo = new VkPipelineDynamicStateCreateInfo();
		m_dynamicCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		m_dynamicCreateInfo->pNext = nullptr;
		m_dynamicCreateInfo->flags = 0;
		m_dynamicCreateInfo->dynamicStateCount = 0;
		m_dynamicCreateInfo->pDynamicStates = nullptr;

		m_graphicsPipelineInfo->pDynamicState = nullptr;
	}

	uint32 numChildren = node.nChildNode();
	for (uint32 childIndex = 0; childIndex < numChildren; childIndex++)
	{
		XMLNode childNode = node.getChildNode(childIndex);
		SetDynamicState(childNode.getName());
	}
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::SetDynamicState(const QuString& stateName)
{
	VkDynamicState state;
	if (stateName == "Viewport")
	{
		state = VK_DYNAMIC_STATE_VIEWPORT;
	}
	else if (stateName == "Scissor")
	{
		state = VK_DYNAMIC_STATE_SCISSOR;
	}
	else if (stateName == "BlendConstants")
	{
		state = VK_DYNAMIC_STATE_BLEND_CONSTANTS;
	}
	else if (stateName == "DepthBias")
	{
		state = VK_DYNAMIC_STATE_DEPTH_BIAS;
	}
	else if (stateName == "DepthBounds")
	{
		state = VK_DYNAMIC_STATE_DEPTH_BOUNDS;
	}
	else if (stateName == "LineWidth")
	{
		state = VK_DYNAMIC_STATE_LINE_WIDTH;
	}
	else if (stateName == "StencilCompareMask")
	{
		state = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
	}
	else if (stateName == "StencilReference")
	{
		state = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
	}
	else if (stateName == "StencilWriteMask")
	{
		state = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
	}
	else
	{
		ERROR_AND_DIE("Dynamic state name not recognized\n");
	}

	m_dynamicStates.push_back(state);

	m_dynamicCreateInfo->dynamicStateCount = m_dynamicStates.size();
	m_dynamicCreateInfo->pDynamicStates = &m_dynamicStates[0];
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseMultisampleStateNode(const XMLNode& node)
{
	if (!m_multisampleCreateInfo)
	{
		m_multisampleCreateInfo = new VkPipelineMultisampleStateCreateInfo();
		m_multisampleCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		m_multisampleCreateInfo->pNext = nullptr;
		m_multisampleCreateInfo->flags = 0;
		m_multisampleCreateInfo->pSampleMask = nullptr;

		m_graphicsPipelineInfo->pMultisampleState = m_multisampleCreateInfo;
	}

	uint32 numChildren = node.nChildNode();
	for (uint32 childIndex = 0; childIndex < numChildren; childIndex++)
	{
		XMLNode childNode = node.getChildNode(childIndex);
		QuString nodeName = childNode.getName();
		if (nodeName == "SampleShading")
		{
			ParseSampleShadingNode(childNode);
		}
		else if (nodeName == "RasterizationSamples")
		{
			ParseRasterizationSamplesNode(childNode);
		}
		else if (nodeName == "Miscellaneous")
		{
			ParseMiscellaneousNode(childNode);
		}
		else
		{
			ERROR_AND_DIE("MultisampleState child node not recognized\n");
		}
	}
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseSampleShadingNode(const XMLNode& node)
{
	XML_EXTRACT_ATTRIBUTE(node, enable);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(enable, m_multisampleCreateInfo->sampleShadingEnable,
		"true", VK_TRUE,
		"false", VK_FALSE,
		"Unrecognized value for SampleShading enable attribute\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, minValue);
	XML_SET_VAR_WITH_IGNORE(minValue, m_multisampleCreateInfo->minSampleShading,
		minValue.AsFloat(),
		"");

	uint32 numChildren = node.nChildNode();
	uint32 sampleMaskIndex = 0;
	for (uint32 childIndex = 0; childIndex < numChildren; childIndex++)
	{
		XMLNode childNode = node.getChildNode(childIndex);
		QuString nodeName = childNode.getName();
		if (nodeName == "SampleMask")
		{
			ParseSampleMaskNode(childNode, sampleMaskIndex++);
		}
		else
		{
			ERROR_AND_DIE("Unrecognized child node of SampleShading\n");
		}
	}
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseSampleMaskNode(const XMLNode& node, uint32 sampleMaskIndex)
{
	if (m_sampleMasks.size() <= sampleMaskIndex)
	{
		m_sampleMasks.resize(sampleMaskIndex + 1);
	}

	uint32* sampleMask = &m_sampleMasks[sampleMaskIndex];

	XML_EXTRACT_ATTRIBUTE(node, value);
	XML_SET_VAR_WITH_IGNORE(value, *sampleMask,
		value.AsInt(),
		"");

	m_multisampleCreateInfo->pSampleMask = &m_sampleMasks[0];
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseRasterizationSamplesNode(const XMLNode& node)
{
	XML_EXTRACT_ATTRIBUTE(node, count);
	XML_SET_VAR_ONE_OPTION_ERROR_AND_IGNORE(count, m_multisampleCreateInfo->rasterizationSamples,
		"1", VK_SAMPLE_COUNT_1_BIT,
		"Unsupported rasterization sample count\n",
		"");
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseMiscellaneousNode(const XMLNode& node)
{
	XML_EXTRACT_ATTRIBUTE(node, alphaToOneEnable);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(alphaToOneEnable, m_multisampleCreateInfo->alphaToOneEnable,
		"true", VK_TRUE,
		"false", VK_FALSE,
		"Unrecognized value for alphaToOneEnable attribute\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, alphaToCoverageEnable);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(alphaToCoverageEnable, m_multisampleCreateInfo->alphaToCoverageEnable,
		"true", VK_TRUE,
		"false", VK_FALSE,
		"Unrecognized value for alphaToCoverageEnable attribute\n",
		"");
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseColorBlendStateNode(const XMLNode& node)
{
	if (!m_colorBlendCreateInfo)
	{
		m_colorBlendCreateInfo = new VkPipelineColorBlendStateCreateInfo();
		m_colorBlendCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		m_colorBlendCreateInfo->pNext = nullptr;
		m_colorBlendCreateInfo->flags = 0;
		m_colorBlendCreateInfo->attachmentCount = 0;
		m_colorBlendCreateInfo->pAttachments = nullptr;

		m_graphicsPipelineInfo->pColorBlendState = m_colorBlendCreateInfo;
	}

	uint32 numChildren = node.nChildNode();
	uint32 attachmentIndex = 0;
	for (uint32 childIndex = 0; childIndex < numChildren; childIndex++)
	{
		XMLNode childNode = node.getChildNode(childIndex);
		QuString nodeName = childNode.getName();
		if (nodeName == "BlendConstants")
		{
			ParseBlendConstantsNode(childNode);
		}
		else if (nodeName == "LogicOp")
		{
			ParseLogicOpNode(childNode);
		}
		else if (nodeName == "AttachmentState")
		{
			ParseAttachmentStateNode(childNode, attachmentIndex++);
		}
		else
		{
			ERROR_AND_DIE("Invalid child node for ColorBlendState\n");
		}
	}
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseBlendConstantsNode(const XMLNode& node)
{
	QuString value = XMLUtils::GetAttribute(node, "value");
	if (value == "")
	{
		return;
	}
	else
	{
		float4 blendConstants = value.AsVec4();
		m_colorBlendCreateInfo->blendConstants[0] = blendConstants.x;
		m_colorBlendCreateInfo->blendConstants[1] = blendConstants.y;
		m_colorBlendCreateInfo->blendConstants[2] = blendConstants.z;
		m_colorBlendCreateInfo->blendConstants[3] = blendConstants.w;
	}
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseLogicOpNode(const XMLNode& node)
{
	XML_EXTRACT_ATTRIBUTE(node, enable);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(enable, m_colorBlendCreateInfo->logicOpEnable,
		"true", VK_TRUE,
		"false", VK_FALSE,
		"Unrecognized value for LogicOp enable attribute\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, op);
	XML_SET_VAR_THREE_OPTIONS_ERROR_AND_IGNORE(op, m_colorBlendCreateInfo->logicOp,
		"noop", VK_LOGIC_OP_NO_OP,
		"and", VK_LOGIC_OP_AND,
		"equivalent", VK_LOGIC_OP_EQUIVALENT,
		"Unsupported logic op\n",
		"");
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseAttachmentStateNode(const XMLNode& node, uint32 attachmentIndex)
{
	if (m_colorBlendAttachmentStates.size() <= attachmentIndex)
	{
		//We need to make a new attachment state
		m_colorBlendAttachmentStates.resize(attachmentIndex + 1);
	}
	VkPipelineColorBlendAttachmentState* currentState = &m_colorBlendAttachmentStates[attachmentIndex];

	uint32 numChildren = node.nChildNode();
	for (uint32 childIndex = 0; childIndex < numChildren; childIndex++)
	{
		XMLNode childNode = node.getChildNode(childIndex);
		QuString nodeName = childNode.getName();
		if (nodeName == "BlendEnable")
		{
			ParseBlendEnableNode(childNode, currentState);
		}
		else if (nodeName == "ColorBlend")
		{
			ParseColorBlendNode(childNode, currentState);
		}
		else if (nodeName == "AlphaBlend")
		{
			ParseAlphaBlendNode(childNode, currentState);
		}
		else
		{
			ERROR_AND_DIE("Unrecognized sub-node for ColorBlendState/AttachmentState\n");
		}
	}

	m_colorBlendCreateInfo->attachmentCount = m_colorBlendAttachmentStates.size();
	m_colorBlendCreateInfo->pAttachments = &m_colorBlendAttachmentStates[0];
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseBlendEnableNode(const XMLNode& node, HephPipelineColorBlendAttachmentState currentState)
{
	XML_EXTRACT_ATTRIBUTE(node, value);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(value, currentState->blendEnable,
		"true", VK_TRUE,
		"false", VK_FALSE,
		"Unrecognized value for BlendEnable value attribute\n",
		"");
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseColorBlendNode(const XMLNode& node, HephPipelineColorBlendAttachmentState currentState)
{
	XML_EXTRACT_ATTRIBUTE(node, blendOp);
	XML_SET_VAR_ONE_OPTION_ERROR_AND_IGNORE(blendOp, currentState->colorBlendOp,
		"add", VK_BLEND_OP_ADD,
		"Unsupported color blend op\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, writeMask);
	XML_SET_VAR_ONE_OPTION_ERROR_AND_IGNORE(writeMask, currentState->colorWriteMask,
		"rgba", VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		"Unsupported color write mask\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, dstFactor);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(dstFactor, currentState->dstColorBlendFactor,
		"srcAlpha", VK_BLEND_FACTOR_SRC_ALPHA,
		"inverseSrcAlpha", VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		"Unsupported dstFactor value\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, srcFactor);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(srcFactor, currentState->srcColorBlendFactor,
		"srcAlpha", VK_BLEND_FACTOR_SRC_ALPHA,
		"inverseSrcAlpha", VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		"Unsupported srcFactor value\n",
		"");
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseAlphaBlendNode(const XMLNode& node, HephPipelineColorBlendAttachmentState currentState)
{
	XML_EXTRACT_ATTRIBUTE(node, blendOp);
	XML_SET_VAR_ONE_OPTION_ERROR_AND_IGNORE(blendOp, currentState->alphaBlendOp,
		"add", VK_BLEND_OP_ADD,
		"Unsupported alpha blend op\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, dstFactor);
	XML_SET_VAR_ONE_OPTION_ERROR_AND_IGNORE(dstFactor, currentState->dstAlphaBlendFactor,
		"one", VK_BLEND_FACTOR_ONE,
		"Unsupported dstFactor value\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, srcFactor);
	XML_SET_VAR_ONE_OPTION_ERROR_AND_IGNORE(srcFactor, currentState->srcAlphaBlendFactor,
		"one", VK_BLEND_FACTOR_ONE,
		"Unsupported srcFactor value\n",
		"");
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseDepthStencilStateNode(const XMLNode& node)
{
	if (!m_depthStencilCreateInfo)
	{
		m_depthStencilCreateInfo = new VkPipelineDepthStencilStateCreateInfo();
		m_depthStencilCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		m_depthStencilCreateInfo->pNext = nullptr;
		m_depthStencilCreateInfo->flags = 0;

		m_graphicsPipelineInfo->pDepthStencilState = m_depthStencilCreateInfo;
	}

	uint32 numChildren = node.nChildNode();
	for (uint32 childIndex = 0; childIndex < numChildren; childIndex++)
	{
		XMLNode childNode = node.getChildNode(childIndex);
		QuString nodeName = childNode.getName();
		if (nodeName == "DepthTest")
		{
			ParseDepthTestNode(childNode);
		}
		else if (nodeName == "DepthBounds")
		{
			ParseDepthBoundsNode(childNode);
		}
		else if (nodeName == "StencilTest")
		{
			ParseStencilTestNode(childNode);
		}
		else
		{
			ERROR_AND_DIE("Unrecognized DepthStencilState child node\n");
		}
	}
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseDepthTestNode(const XMLNode& node)
{
	XML_EXTRACT_ATTRIBUTE(node, testEnable);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(testEnable, m_depthStencilCreateInfo->depthTestEnable,
		"true", VK_TRUE,
		"false", VK_FALSE,
		"Unsupported testEnable value\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, writeEnable);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(writeEnable, m_depthStencilCreateInfo->depthWriteEnable,
		"true", VK_TRUE,
		"false", VK_FALSE,
		"Unsupported writeEnable value\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, compareOp);
	XML_SET_VAR_FOUR_OPTIONS_ERROR_AND_IGNORE(compareOp, m_depthStencilCreateInfo->depthCompareOp,
		"lessEqual", VK_COMPARE_OP_LESS_OR_EQUAL,
		"greaterEqual", VK_COMPARE_OP_GREATER_OR_EQUAL,
		"always", VK_COMPARE_OP_ALWAYS,
		"equal", VK_COMPARE_OP_EQUAL,
		"Unsupported compareOp value\n",
		"");
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseDepthBoundsNode(const XMLNode& node)
{
	XML_EXTRACT_ATTRIBUTE(node, enable);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(enable, m_depthStencilCreateInfo->depthBoundsTestEnable,
		"true", VK_TRUE,
		"false", VK_FALSE,
		"Unsupported depth bounds test enable value\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, min);
	XML_SET_VAR_WITH_IGNORE(min, m_depthStencilCreateInfo->minDepthBounds,
		min.AsFloat(),
		"");

	XML_EXTRACT_ATTRIBUTE(node, max);
	XML_SET_VAR_WITH_IGNORE(max, m_depthStencilCreateInfo->maxDepthBounds,
		max.AsFloat(),
		"");
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseStencilTestNode(const XMLNode& node)
{
	XML_EXTRACT_ATTRIBUTE(node, enable);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(enable, m_depthStencilCreateInfo->stencilTestEnable,
		"true", VK_TRUE,
		"false", VK_FALSE,
		"Unsupported stencil test enable value\n",
		"");

	uint32 numChildren = node.nChildNode();
	for (uint32 childIndex = 0; childIndex < numChildren; childIndex++)
	{
		XMLNode childNode = node.getChildNode(childIndex);
		QuString nodeName = childNode.getName();
		if (nodeName == "BackStencil")
		{
			ParseStencilNode(childNode, &m_depthStencilCreateInfo->back);
		}
		else if (nodeName == "FrontStencil")
		{
			ParseStencilNode(childNode, &m_depthStencilCreateInfo->front);
		}
		else
		{
			ERROR_AND_DIE("Unrecognized StencilTest child node\n");
		}
	}
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseStencilNode(const XMLNode& node, HephStencilOpState stencil)
{
	XML_EXTRACT_ATTRIBUTE(node, compareMask);
	XML_SET_VAR_WITH_IGNORE(compareMask, stencil->compareMask,
		compareMask.AsInt(),
		"");

	XML_EXTRACT_ATTRIBUTE(node, compareOp);
	XML_SET_VAR_ONE_OPTION_ERROR_AND_IGNORE(compareOp, stencil->compareOp,
		"always", VK_COMPARE_OP_ALWAYS,
		"Unsupported stencil compareOp value\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, depthFailOp);
	XML_SET_VAR_THREE_OPTIONS_ERROR_AND_IGNORE(depthFailOp, stencil->depthFailOp,
		"keep", VK_STENCIL_OP_KEEP,
		"replace", VK_STENCIL_OP_REPLACE,
		"incrementClamp", VK_STENCIL_OP_INCREMENT_AND_CLAMP,
		"Unsupported depth fail op value\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, failOp);
	XML_SET_VAR_THREE_OPTIONS_ERROR_AND_IGNORE(failOp, stencil->failOp,
		"keep", VK_STENCIL_OP_KEEP,
		"replace", VK_STENCIL_OP_REPLACE,
		"incrementClamp", VK_STENCIL_OP_INCREMENT_AND_CLAMP,
		"Unsupported stencil test fail op value\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, passOp);
	XML_SET_VAR_THREE_OPTIONS_ERROR_AND_IGNORE(passOp, stencil->passOp,
		"keep", VK_STENCIL_OP_KEEP,
		"replace", VK_STENCIL_OP_REPLACE,
		"incrementClamp", VK_STENCIL_OP_INCREMENT_AND_CLAMP,
		"Unsupported stencil test pass op value\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, reference);
	XML_SET_VAR_WITH_IGNORE(reference, stencil->reference,
		reference.AsInt(),
		"");

	XML_EXTRACT_ATTRIBUTE(node, writeMask);
	XML_SET_VAR_WITH_IGNORE(writeMask, stencil->writeMask,
		writeMask.AsInt(),
		"");
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseTessellationStateNode(const XMLNode& node)
{
	if (!m_tessellationCreateInfo)
	{
		m_tessellationCreateInfo = new VkPipelineTessellationStateCreateInfo();
		m_tessellationCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		m_tessellationCreateInfo->pNext = nullptr;
		m_tessellationCreateInfo->flags = 0;

		m_graphicsPipelineInfo->pTessellationState = nullptr;
	}

	XML_EXTRACT_ATTRIBUTE(node, patchControlPoints);
	XML_SET_VAR_WITH_IGNORE(patchControlPoints, m_tessellationCreateInfo->patchControlPoints,
		patchControlPoints.AsInt(),
		"");
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseViewportStateNode(const XMLNode& node)
{
	if (!m_viewportCreateInfo)
	{
		m_viewportCreateInfo = new VkPipelineViewportStateCreateInfo();
		m_viewportCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		m_viewportCreateInfo->pNext = nullptr;
		m_viewportCreateInfo->flags = 0;

		m_graphicsPipelineInfo->pViewportState = m_viewportCreateInfo;
	}

	uint32 numChildren = node.nChildNode();
	uint32 viewportIndex = 0;
	uint32 scissorIndex = 0;
	for (uint32 childIndex = 0; childIndex < numChildren; childIndex++)
	{
		XMLNode childNode = node.getChildNode(childIndex);
		QuString nodeName = childNode.getName();
		if (nodeName == "Viewport")
		{
			ParseViewportNode(childNode, viewportIndex++);
		}
		else if (nodeName == "Scissor")
		{
			ParseScissorNode(childNode, scissorIndex++);
		}
		else
		{
			ERROR_AND_DIE("Unrecognized child node of ViewportState\n");
		}
	}
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseViewportNode(const XMLNode& node, uint32 viewportIndex)
{
	if (m_viewports.size() <= viewportIndex)
	{
		m_viewports.resize(viewportIndex + 1);
	}
	VkViewport* viewport = &m_viewports[viewportIndex];

	XML_EXTRACT_ATTRIBUTE(node, minDepth);
	XML_SET_VAR_WITH_IGNORE(minDepth, viewport->minDepth,
		minDepth.AsFloat(),
		"");

	XML_EXTRACT_ATTRIBUTE(node, maxDepth);
	XML_SET_VAR_WITH_IGNORE(maxDepth, viewport->maxDepth,
		maxDepth.AsFloat(),
		"");

	XML_EXTRACT_ATTRIBUTE(node, width);
	XML_SET_VAR_ONE_OPTION_ERROR_AND_IGNORE(width, viewport->width,
		"full", (float)HManager::GetWindowWidth(),
		"Unsupported viewport width\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, height);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(height, viewport->height,
		"full", (float)HManager::GetWindowHeight(),
		"-full", -(float)HManager::GetWindowHeight(),
		"Unsupported viewport height\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, topLeft);
	float2 vTopLeft = topLeft.AsVec2();
	XML_SET_VAR_WITH_IGNORE(topLeft, viewport->x, vTopLeft.x, "");
	XML_SET_VAR_WITH_IGNORE(topLeft, viewport->y, vTopLeft.y, "");

	m_viewportCreateInfo->viewportCount = m_viewports.size();
	m_viewportCreateInfo->pViewports = &m_viewports[0];
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseScissorNode(const XMLNode& node, uint32 scissorIndex)
{
	if (m_scissors.size() <= scissorIndex)
	{
		m_scissors.resize(scissorIndex + 1);
	}

	VkRect2D* scissor = &m_scissors[scissorIndex];

	XML_EXTRACT_ATTRIBUTE(node, topLeft);
	float2 vTopLeft = topLeft.AsVec2();
	XML_SET_VAR_WITH_IGNORE(topLeft, scissor->offset.x, (uint32)vTopLeft.x, "");
	XML_SET_VAR_WITH_IGNORE(topLeft, scissor->offset.y, (uint32)vTopLeft.y, "");

	XML_EXTRACT_ATTRIBUTE(node, width);
	XML_SET_VAR_ONE_OPTION_ERROR_AND_IGNORE(width, scissor->extent.width,
		"full", HManager::GetWindowWidth(),
		"Unsupported scissor width value\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, height);
	XML_SET_VAR_ONE_OPTION_ERROR_AND_IGNORE(height, scissor->extent.height,
		"full", HManager::GetWindowHeight(),
		"Unsupported scissor height value\n",
		"");

	m_viewportCreateInfo->scissorCount = m_scissors.size();
	m_viewportCreateInfo->pScissors = &m_scissors[0];
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseRasterizationStateNode(const XMLNode& node)
{
	if (!m_rasterizationCreateInfo)
	{
		m_rasterizationCreateInfo = new VkPipelineRasterizationStateCreateInfo();
		m_rasterizationCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		m_rasterizationCreateInfo->pNext = nullptr;
		m_rasterizationCreateInfo->flags = 0;

		m_graphicsPipelineInfo->pRasterizationState = m_rasterizationCreateInfo;
	}

	uint32 numChildren = node.nChildNode();
	for (uint32 childIndex = 0; childIndex < numChildren; childIndex++)
	{
		XMLNode childNode = node.getChildNode(childIndex);
		QuString nodeName = childNode.getName();

		if (nodeName == "DepthBias")
		{
			ParseDepthBiasNode(childNode);
		}
		else if (nodeName == "RasterizationMode")
		{
			ParseRasterizationModeNode(childNode);
		}
		else
		{
			ERROR_AND_DIE("Unrecognized child node of RasterizationState\n");
		}
	}
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseDepthBiasNode(const XMLNode& node)
{
	XML_EXTRACT_ATTRIBUTE(node, enable);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(enable, m_rasterizationCreateInfo->depthBiasEnable,
		"true", VK_TRUE,
		"false", VK_FALSE,
		"Unsupported depth bias enable value\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, clampEnable);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(clampEnable, m_rasterizationCreateInfo->depthClampEnable,
		"true", VK_TRUE,
		"false", VK_FALSE,
		"Unsupported depth clamp enable value\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, clamp);
	XML_SET_VAR_WITH_IGNORE(clamp, m_rasterizationCreateInfo->depthBiasClamp,
		clamp.AsFloat(),
		"");

	XML_EXTRACT_ATTRIBUTE(node, constantFactor);
	XML_SET_VAR_WITH_IGNORE(constantFactor, m_rasterizationCreateInfo->depthBiasConstantFactor,
		constantFactor.AsFloat(),
		"");

	XML_EXTRACT_ATTRIBUTE(node, slopeFactor);
	XML_SET_VAR_WITH_IGNORE(slopeFactor, m_rasterizationCreateInfo->depthBiasSlopeFactor,
		slopeFactor.AsFloat(),
		"");
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ParseRasterizationModeNode(const XMLNode& node)
{
	XML_EXTRACT_ATTRIBUTE(node, cullMode);
	XML_SET_VAR_FOUR_OPTIONS_ERROR_AND_IGNORE(cullMode, m_rasterizationCreateInfo->cullMode,
		"backFace", VK_CULL_MODE_BACK_BIT,
		"frontFace", VK_CULL_MODE_FRONT_BIT,
		"none", VK_CULL_MODE_NONE,
		"both", VK_CULL_MODE_FRONT_AND_BACK,
		"Unsupported cull mode value\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, frontFace);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(frontFace, m_rasterizationCreateInfo->frontFace,
		"cw", VK_FRONT_FACE_CLOCKWISE,
		"ccw", VK_FRONT_FACE_COUNTER_CLOCKWISE,
		"Unsupported front face (winding order) value\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, polyMode);
	XML_SET_VAR_ONE_OPTION_ERROR_AND_IGNORE(polyMode, m_rasterizationCreateInfo->polygonMode,
		"fill", VK_POLYGON_MODE_FILL,
		"Unsupported polygon rasterization mode\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, discardEnable);
	XML_SET_VAR_TWO_OPTIONS_ERROR_AND_IGNORE(discardEnable, m_rasterizationCreateInfo->rasterizerDiscardEnable,
		"true", VK_TRUE,
		"false", VK_FALSE,
		"Unsupported rasterizer discard enable value\n",
		"");

	XML_EXTRACT_ATTRIBUTE(node, lineWidth);
	XML_SET_VAR_WITH_IGNORE(lineWidth, m_rasterizationCreateInfo->lineWidth,
		lineWidth.AsFloat(),
		"");
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::InitDefaultGraphicsPipeline()
{
	m_graphicsPipelineInfo = new VkGraphicsPipelineCreateInfo();

	m_graphicsPipelineInfo->sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	m_graphicsPipelineInfo->pNext = nullptr;
	m_graphicsPipelineInfo->flags = 0; //Derivative pipelines not existing yet.  We'll see if there's a good way to expose this later
	m_graphicsPipelineInfo->basePipelineIndex = H_INVALID_INDEX;
	m_graphicsPipelineInfo->basePipelineHandle = H_NULL_HANDLE;
	m_graphicsPipelineInfo->renderPass = H_NULL_HANDLE; //Expect to be assigned a render pass for this pipeline

	m_graphicsPipelineInfo->stageCount = H_INVALID;
	m_graphicsPipelineInfo->pStages = nullptr;

	//We'll create the layout as part of shader finalization
	//as we need to know push constants, uniform buffers,
	//shader storage blocks, samplers, etc.
	m_graphicsPipelineInfo->layout = H_NULL_HANDLE;

	//Not ready for this, as it's part of shader binding
	//Intending to assign a vertex type and enforce location order
	m_graphicsPipelineInfo->pVertexInputState = nullptr;

	//Not supporting dynamic states at this time
	m_graphicsPipelineInfo->pDynamicState = nullptr;

	InitDefaultMultisamplingState();

	InitDefaultColorBlendState();

	InitDefaultDepthStencilState();

	InitDefaultTessellationState();

	InitDefaultViewportState();

	InitDefaultInputAssemblyState();

	InitDefaultRasterizationState();
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::InitDefaultComputePipeline()
{
	m_computePipelineInfo = new VkComputePipelineCreateInfo();

	m_computePipelineInfo->sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	m_computePipelineInfo->pNext = nullptr;
	m_computePipelineInfo->flags = 0; //Same as above, no derivative pipelines
	m_computePipelineInfo->basePipelineHandle = H_NULL_HANDLE;
	m_computePipelineInfo->basePipelineIndex = H_INVALID_INDEX;
	m_computePipelineInfo->layout = H_NULL_HANDLE; //Must populate as part of shader binding

	//Since stage create info is stored on the shader object, I'm just setting this to nothing
	m_computePipelineInfo->stage = {};
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::InitDefaultRasterizationState()
{
	m_rasterizationCreateInfo = new VkPipelineRasterizationStateCreateInfo();
	m_rasterizationCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	m_rasterizationCreateInfo->pNext = nullptr;
	m_rasterizationCreateInfo->flags = 0;
	m_rasterizationCreateInfo->cullMode = VK_CULL_MODE_NONE;
	m_rasterizationCreateInfo->depthBiasEnable = VK_FALSE;
	m_rasterizationCreateInfo->depthBiasClamp = 0.f;
	m_rasterizationCreateInfo->depthBiasConstantFactor = 0.f;
	m_rasterizationCreateInfo->depthBiasSlopeFactor = 0.f;
	m_rasterizationCreateInfo->depthClampEnable = VK_FALSE;
	m_rasterizationCreateInfo->frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //GL rules
	m_rasterizationCreateInfo->lineWidth = 1.f;
	m_rasterizationCreateInfo->polygonMode = VK_POLYGON_MODE_FILL; //Default rasterization
	m_rasterizationCreateInfo->rasterizerDiscardEnable = VK_FALSE; //Perhaps if we want to throw away primitives before a fragment shader?

	m_graphicsPipelineInfo->pRasterizationState = m_rasterizationCreateInfo;
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::InitDefaultInputAssemblyState()
{
	m_inputAssemblyCreateInfo = new VkPipelineInputAssemblyStateCreateInfo();
	m_inputAssemblyCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_inputAssemblyCreateInfo->pNext = nullptr;
	m_inputAssemblyCreateInfo->flags = 0;
	m_inputAssemblyCreateInfo->primitiveRestartEnable = VK_FALSE; //Not allowed for the standard triangle list, so disabling.  Cool feature, though
	m_inputAssemblyCreateInfo->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; //For obvious reasons

	m_graphicsPipelineInfo->pInputAssemblyState = m_inputAssemblyCreateInfo;
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::InitDefaultViewportState()
{
	m_viewportCreateInfo = new VkPipelineViewportStateCreateInfo();
	m_viewportCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	m_viewportCreateInfo->pNext = nullptr;
	m_viewportCreateInfo->flags = 0;

	m_viewports.resize(1);
	VkViewport* viewport = &m_viewports[0];
	viewport->minDepth = 0.f;
	viewport->maxDepth = 1.f;
	viewport->width = (float)HManager::GetWindowWidth();
	viewport->height = -(float)HManager::GetWindowHeight();
	viewport->x = 0.f;
	viewport->y = 0.f;

	m_viewportCreateInfo->viewportCount = m_viewports.size();
	m_viewportCreateInfo->pViewports = &m_viewports[0];

	m_scissors.resize(1);
	VkRect2D* scissor = &m_scissors[0];
	scissor->offset.x = 0;
	scissor->offset.y = 0;
	scissor->extent.width = HManager::GetWindowWidth();
	scissor->extent.height = HManager::GetWindowHeight();

	m_viewportCreateInfo->scissorCount = m_scissors.size();
	m_viewportCreateInfo->pScissors = &m_scissors[0];

	m_graphicsPipelineInfo->pViewportState = m_viewportCreateInfo;
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::InitDefaultTessellationState()
{
	//Making an invalid tessellation state, because if we bind a tessellation shader, I expect this info to be updated
	m_tessellationCreateInfo = new VkPipelineTessellationStateCreateInfo();
	m_tessellationCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	m_tessellationCreateInfo->pNext = nullptr;
	m_tessellationCreateInfo->flags = 0;
	m_tessellationCreateInfo->patchControlPoints = 0;

	//Since tessellation is a non-standard state to need, we'll also set the pipeline's pointer for it to null
	//This needs to be updated on a successful tessellation state change
	m_graphicsPipelineInfo->pTessellationState = nullptr;
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::InitDefaultDepthStencilState()
{
	m_depthStencilCreateInfo = new VkPipelineDepthStencilStateCreateInfo();
	m_depthStencilCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	m_depthStencilCreateInfo->pNext = nullptr;
	m_depthStencilCreateInfo->flags = 0;
	m_depthStencilCreateInfo->depthTestEnable = VK_TRUE;
	m_depthStencilCreateInfo->depthWriteEnable = VK_TRUE;
	m_depthStencilCreateInfo->depthCompareOp = VK_COMPARE_OP_ALWAYS;
	m_depthStencilCreateInfo->depthBoundsTestEnable = VK_FALSE;
	m_depthStencilCreateInfo->minDepthBounds = 0.f;
	m_depthStencilCreateInfo->maxDepthBounds = 1.f;

	//Stencil testing is not default behavior, so I won't even set those up
	m_depthStencilCreateInfo->stencilTestEnable = VK_FALSE;

	m_graphicsPipelineInfo->pDepthStencilState = m_depthStencilCreateInfo;
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::InitDefaultColorBlendState()
{
	m_colorBlendCreateInfo = new VkPipelineColorBlendStateCreateInfo();
	m_colorBlendCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	m_colorBlendCreateInfo->pNext = nullptr;
	float blendConsts[4] = { 1.f, 1.f, 1.f, 1.f };	//Used if a blend factor uses constant colors
	memcpy(m_colorBlendCreateInfo->blendConstants, blendConsts, sizeof(blendConsts));
	m_colorBlendCreateInfo->logicOpEnable = VK_FALSE;
	m_colorBlendCreateInfo->logicOp = VK_LOGIC_OP_NO_OP; //It's easier to expose this if we don't put a double barrier in place

	//Setting a default alpha blending state for one color attachment (normal usage)
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	m_colorBlendAttachmentStates.push_back(colorBlendAttachment);

	//-----------------------------------------------------------------------------------------------
	//Important to reset these when any change to the vector size may occur
	//-----------------------------------------------------------------------------------------------
	m_colorBlendCreateInfo->attachmentCount = m_colorBlendAttachmentStates.size();
	m_colorBlendCreateInfo->pAttachments = m_colorBlendAttachmentStates.data();

	m_graphicsPipelineInfo->pColorBlendState = m_colorBlendCreateInfo;
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::InitDefaultMultisamplingState()
{
	//Multisampling was a little murky, so I'm using default values
	m_multisampleCreateInfo = new VkPipelineMultisampleStateCreateInfo();
	m_multisampleCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_multisampleCreateInfo->pNext = nullptr;
	m_multisampleCreateInfo->flags = 0;
	m_multisampleCreateInfo->alphaToCoverageEnable = VK_FALSE;
	m_multisampleCreateInfo->alphaToOneEnable = VK_FALSE;
	m_multisampleCreateInfo->minSampleShading = 1.f;
	m_multisampleCreateInfo->pSampleMask = nullptr;
	m_multisampleCreateInfo->rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	m_multisampleCreateInfo->sampleShadingEnable = VK_FALSE;

	m_graphicsPipelineInfo->pMultisampleState = m_multisampleCreateInfo;
}


//-----------------------------------------------------------------------------------------------
void HPipelineGenerator::ReleaseMemory()
{
	m_sampleMasks.clear();
	m_renderPass = H_NULL_HANDLE;
	m_descriptorSetLayouts.clear();
	m_graphicsShaders.clear();
	m_colorBlendAttachmentStates.clear();
	m_dynamicStates.clear();
	m_scissors.clear();
	m_viewports.clear();

	SAFE_DELETE(m_rasterizationCreateInfo);
	SAFE_DELETE(m_inputAssemblyCreateInfo);
	SAFE_DELETE(m_viewportCreateInfo);
	SAFE_DELETE(m_multisampleCreateInfo);
	SAFE_DELETE(m_colorBlendCreateInfo);
	SAFE_DELETE(m_depthStencilCreateInfo);
	SAFE_DELETE(m_dynamicCreateInfo);
	SAFE_DELETE(m_tessellationCreateInfo);

	SAFE_DELETE(m_graphicsPipelineInfo);
	SAFE_DELETE(m_computePipelineInfo);
}
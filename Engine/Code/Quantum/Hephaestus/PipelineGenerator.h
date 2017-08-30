#pragma once

#include "Quantum/Hephaestus/Declarations.h"

#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
struct HDescriptor
{
	uint32 bindingPoint;
	EUniformDescriptorType type;
	HShaderTypeBits shaderStage;
};


//-----------------------------------------------------------------------------------------------
struct HReflectionData
{
	std::map<uint32, HDescriptor> descriptors;
	uint32 currentBindingIndex = 0;
};


//-----------------------------------------------------------------------------------------------
class HPipelineGenerator
{
	friend class HManager;

private:
	HPipelineGenerator();
	~HPipelineGenerator();

public:
	class HPipeline* GenerateGraphicsPipeline();

	void Reset();
	void AddShader(class HShader* shader, bool isCompute = false);
	void SetVertexType(EVertexType vertexType);
	void AssignSubpass(struct HSubpass* subpass);
	void AddDescriptorSetLayout(HephDescriptorSetLayout layout) { m_descriptorSetLayouts.push_back(layout); }
	void InitializeFromXML(const class QuString& pipelineName);
	HPipeline* CreateOrGetPipeline(const QuString& name, uint32 renderPassName, uint32 subpassIndex);


private:
	void InitializeFromXML(const QuString& pipelineName, uint32 level, uint32& lastShaderLevelWritten);
	void InitializeGraphicsPipelineFromXML(const struct XMLNode &rootNode, uint32 level, uint32& lastShaderLevelWritten);
	void ParseShaderNode(const XMLNode& node, uint32 level, uint32& lastShaderLevelWritten);
	void ParseVertexStateNode(const XMLNode& node);
	void ParseVertexTypeNode(const XMLNode& node);
	void ParseInputAssemblyNode(const XMLNode& node);
	void ParseDynamicStateNode(const XMLNode& node);
	void SetDynamicState(const class QuString& stateName);
	void ParseMultisampleStateNode(const XMLNode& node);
	void ParseSampleShadingNode(const XMLNode& node);
	void ParseSampleMaskNode(const XMLNode& node, uint32 sampleMaskIndex);
	void ParseRasterizationSamplesNode(const XMLNode& node);
	void ParseMiscellaneousNode(const XMLNode& node);
	void ParseColorBlendStateNode(const XMLNode& node);
	void ParseBlendConstantsNode(const XMLNode& node);
	void ParseLogicOpNode(const XMLNode& node);
	void ParseAttachmentStateNode(const XMLNode& node, uint32 attachmentIndex);
	void ParseBlendEnableNode(const XMLNode& node, HephPipelineColorBlendAttachmentState currentState);
	void ParseColorBlendNode(const XMLNode& node, HephPipelineColorBlendAttachmentState currentState);
	void ParseAlphaBlendNode(const XMLNode& node, HephPipelineColorBlendAttachmentState currentState);
	void ParseDepthStencilStateNode(const XMLNode& node);
	void ParseDepthTestNode(const XMLNode& node);
	void ParseDepthBoundsNode(const XMLNode& node);
	void ParseStencilTestNode(const XMLNode& node);
	void ParseStencilNode(const XMLNode& node, HephStencilOpState stencil);
	void ParseTessellationStateNode(const XMLNode& node);
	void ParseViewportStateNode(const XMLNode& node);
	void ParseViewportNode(const XMLNode& node, uint32 viewportIndex);
	void ParseScissorNode(const XMLNode& node, uint32 scissorIndex);
	void ParseRasterizationStateNode(const XMLNode& node);
	void ParseDepthBiasNode(const XMLNode& node);
	void ParseRasterizationModeNode(const XMLNode& node);
	void InitDefaultGraphicsPipeline();
	void InitDefaultComputePipeline();

	void ReleaseMemory();
	void InitDefaultRasterizationState();
	void InitDefaultInputAssemblyState();
	void InitDefaultViewportState();
	void InitDefaultTessellationState();
	void InitDefaultDepthStencilState();
	void InitDefaultColorBlendState();
	void InitDefaultMultisamplingState();

private:
	HephGraphicsPipelineCreateInfo m_graphicsPipelineInfo = nullptr;
	HephComputePipelineCreateInfo m_computePipelineInfo = nullptr;
	HephPipelineCache m_cache = H_NULL_HANDLE;

private:
	//Graphics pipeline cached create infos
	HephPipelineMultisampleStateCreateInfo m_multisampleCreateInfo = nullptr;
	HephPipelineColorBlendStateCreateInfo m_colorBlendCreateInfo = nullptr;
	HephPipelineDepthStencilStateCreateInfo m_depthStencilCreateInfo = nullptr;
	HephPipelineTessellationStateCreateInfo m_tessellationCreateInfo = nullptr;
	HephPipelineViewportStateCreateInfo m_viewportCreateInfo = nullptr;
	HephPipelineInputAssemblyStateCreateInfo m_inputAssemblyCreateInfo = nullptr;
	HephPipelineRasterizationStateCreateInfo m_rasterizationCreateInfo = nullptr;
	HephPipelineDynamicStateCreateInfo m_dynamicCreateInfo = nullptr;
	std::vector<HephDynamicState> m_dynamicStates;
	std::vector<HephViewport_T> m_viewports;
	std::vector<HephRect2D_T> m_scissors;
	HephRenderPass m_renderPass = H_NULL_HANDLE;
	std::vector<uint32> m_sampleMasks;

	std::vector<HephDescriptorSetLayout> m_descriptorSetLayouts;

	
	std::vector<HShader*> m_graphicsShaders;
	std::vector<HephPipelineColorBlendAttachmentState_T> m_colorBlendAttachmentStates;

	std::map<uint32, HPipeline*> m_pipelines;
};
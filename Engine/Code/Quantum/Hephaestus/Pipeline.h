#pragma once

#include "Quantum/Hephaestus/Declarations.h"
#include "Quantum/Hephaestus/PipelineGenerator.h"

#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
class HPipeline
{
	friend class HPipelineGenerator;
	friend class HMaterialState;
	
public:
	operator HephPipeline() const { return m_pipeline; }
	uint32 GetSubpassIndex() const { return m_subpassIndex; }

private:
	HPipeline(const HephGraphicsPipelineCreateInfo createInfo, HephPipelineCache cache, const HReflectionData& reflectionData);
	void CreateOrGetLayout();

public:
	~HPipeline();

private:
	static std::map<uint32, HephPipelineLayout> s_registeredLayouts;
	std::vector<HephDescriptorSetLayout> m_descriptorSetLayouts;

private:
	HReflectionData m_pipelineReflection;
	HephPipelineLayout m_layout;
	HephPipeline m_pipeline;
	uint32 m_subpassIndex;
};
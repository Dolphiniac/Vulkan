#pragma once

#include "Quantum/Hephaestus/Declarations.h"

#include <vector>


//-----------------------------------------------------------------------------------------------
class HDescriptorSetLayoutGenerator
{
public:
	void ClearBindings();
	void Bind(HShaderTypes validShaderStages, uint32 bindingIndex, EUniformDescriptorType type);
	HephDescriptorSetLayout GetLayout();

private:
	HephDescriptorSetLayout m_currentLayout = H_NULL_HANDLE;
	std::vector<HephDescriptorSetLayoutBinding_T> m_bindingDescriptions;
};
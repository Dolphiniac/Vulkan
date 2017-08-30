#include "Quantum/Hephaestus/DescriptorSetLayoutGenerator.h"
#include "Quantum/Hephaestus/Manager.h"
#include "Quantum/Hephaestus/LogicalDevice.h"

#include <vulkan.h>


//-----------------------------------------------------------------------------------------------
void HDescriptorSetLayoutGenerator::ClearBindings()
{
	m_bindingDescriptions.clear();
	m_currentLayout = H_NULL_HANDLE;
}


//-----------------------------------------------------------------------------------------------
void HDescriptorSetLayoutGenerator::Bind(HShaderTypes validShaderStages, uint32 bindingIndex, EUniformDescriptorType type)
{
	HephDescriptorSetLayoutBinding_T binding;
	binding.binding = bindingIndex;
	binding.descriptorCount = 1;

	switch (type)
	{
	case H_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		break;
	case H_DESCRIPTOR_TYPE_SAMPLER2D:
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		break;
	default:
		ERROR_AND_DIE("Unsupported descriptor type\n");
	}

	uint32 shaderStageFlags = 0;
	shaderStageFlags += (validShaderStages & H_SHADER_TYPE_VERTEX_BIT) > 0 ? VK_SHADER_STAGE_VERTEX_BIT : 0;
	shaderStageFlags += (validShaderStages & H_SHADER_TYPE_FRAGMENT_BIT) > 0 ? VK_SHADER_STAGE_FRAGMENT_BIT : 0;
	shaderStageFlags += (validShaderStages & H_SHADER_TYPE_TESSELLATION_CONTROL_BIT) > 0 ? VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT : 0;
	shaderStageFlags += (validShaderStages & H_SHADER_TYPE_TESSELLATION_EVALUATION_BIT) > 0 ? VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT : 0;
	shaderStageFlags += (validShaderStages & H_SHADER_TYPE_GEOMETRY_BIT) > 0 ? VK_SHADER_STAGE_GEOMETRY_BIT : 0;
	shaderStageFlags += (validShaderStages & H_SHADER_TYPE_COMPUTE_BIT) > 0 ? VK_SHADER_STAGE_COMPUTE_BIT : 0;

	binding.stageFlags = shaderStageFlags;

	binding.pImmutableSamplers = nullptr;

	m_bindingDescriptions.push_back(std::move(binding));

	m_currentLayout = H_NULL_HANDLE;	//Can't reuse this layout, since it's changed
}


//-----------------------------------------------------------------------------------------------
HephDescriptorSetLayout HDescriptorSetLayoutGenerator::GetLayout()
{
	if (m_currentLayout == H_NULL_HANDLE)
	{
		VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo;
		setLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setLayoutCreateInfo.pNext = nullptr;
		setLayoutCreateInfo.flags = 0;
		setLayoutCreateInfo.bindingCount = m_bindingDescriptions.size();
		setLayoutCreateInfo.pBindings = &m_bindingDescriptions[0];

		HLogicalDevice* device = HManager::GetLogicalDevice();

		H_ASSERT(vkCreateDescriptorSetLayout(*device, &setLayoutCreateInfo, nullptr, &m_currentLayout), "Could not create descriptor set layout\n");
	}

	return m_currentLayout;
}
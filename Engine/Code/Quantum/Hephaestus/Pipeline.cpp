#include "Quantum/Hephaestus/Pipeline.h"
#include "Quantum/Hephaestus/Manager.h"
#include "Quantum/Hephaestus/LogicalDevice.h"
#include "Quantum/Math/Noise.h"
#include "Quantum/Hephaestus/BufferDescriptor.h"

#include <vulkan.h>


//-----------------------------------------------------------------------------------------------
STATIC std::map<uint32, HephPipelineLayout> HPipeline::s_registeredLayouts;


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HPipeline::HPipeline(const HephGraphicsPipelineCreateInfo createInfo, HephPipelineCache cache, const HReflectionData& reflectionData)
	: m_pipelineReflection(reflectionData)
	, m_subpassIndex(createInfo->subpass)
{
	//We know that we'll only use two descriptor set layouts, one for the global set, and one for the local set
	m_descriptorSetLayouts.resize(2);
	HUniform::GetDescriptorSetAndLayout(&m_descriptorSetLayouts[0], nullptr);

	HLogicalDevice* device = HManager::GetLogicalDevice();
	if (reflectionData.descriptors.empty())
	{
		m_descriptorSetLayouts.resize(1);
	}
	else
	{
		VkDescriptorSetLayoutCreateInfo localLayoutCreateInfo;
		localLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		localLayoutCreateInfo.pNext = nullptr;
		localLayoutCreateInfo.flags = 0;

		uint32 numDescriptors = reflectionData.descriptors.size();
		localLayoutCreateInfo.bindingCount = numDescriptors;
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		auto descriptorIter = reflectionData.descriptors.begin();
		for (uint32 bindingIndex = 0; bindingIndex < numDescriptors; bindingIndex++)
		{
			VkDescriptorSetLayoutBinding binding;
			binding.descriptorCount = 1;	//Enforcing no arrays of buffers
			binding.pImmutableSamplers = nullptr;
			const HDescriptor& descriptor = descriptorIter->second;
			switch (descriptor.shaderStage)
			{
			case H_SHADER_TYPE_VERTEX_BIT:
				binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				break;
			case H_SHADER_TYPE_FRAGMENT_BIT:
				binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				break;
			case H_SHADER_TYPE_COMPUTE_BIT:
				binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
				break;
			case H_SHADER_TYPE_GEOMETRY_BIT:
				binding.stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT;
				break;
			case H_SHADER_TYPE_TESSELLATION_CONTROL_BIT:
				binding.stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
				break;
			case H_SHADER_TYPE_TESSELLATION_EVALUATION_BIT:
				binding.stageFlags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
				break;
			default:
				ERROR_AND_DIE("We literally can't hit this\n");
				break;
			}
			binding.binding = descriptor.bindingPoint;
			switch (descriptor.type)
			{
			case H_DESCRIPTOR_TYPE_SAMPLER2D:
				binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				break;
			case H_DESCRIPTOR_TYPE_SAMPLER:
				binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				break;
			case H_DESCRIPTOR_TYPE_TEXTURE2D:
				binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				break;
			case H_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				break;
			case H_DESCRIPTOR_TYPE_SHADER_STORAGE_BUFFER:
				binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				break;
			case H_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
				binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
				break;
			case H_DESCRIPTOR_TYPE_TEXEL_BUFFER:
				binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
				break;
			default:
				ERROR_AND_DIE("Somehow, we broke this\n");
				break;
			}

			bindings.push_back(std::move(binding));
			descriptorIter++;
		}

		localLayoutCreateInfo.pBindings = &bindings[0];

		H_ASSERT(vkCreateDescriptorSetLayout(*device, &localLayoutCreateInfo, nullptr, &m_descriptorSetLayouts[1]), "Could not create shader-local descriptor set layout\n");
	}


	//Then, we can either retrieve an existing pipeline layout or create a new one
	CreateOrGetLayout();

	createInfo->layout = m_layout;


	H_ASSERT(vkCreateGraphicsPipelines(*device, cache, 1, createInfo, nullptr, &m_pipeline), "Could not create graphics pipeline\n");

	createInfo->layout = H_NULL_HANDLE;
}


//-----------------------------------------------------------------------------------------------
HPipeline::~HPipeline()
{
	HLogicalDevice* device = HManager::GetLogicalDevice();

	vkDestroyPipeline(*device, m_pipeline, nullptr);

	vkDestroyPipelineLayout(*device, m_layout, nullptr);
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
void HPipeline::CreateOrGetLayout()
{
	//We don't want to create unique layouts for EVERY pipeline if we can find an existing, compatible one
	uint32 hashResult = 0xDEADBEEF;

	for (uint32 descriptorIndex = 0; descriptorIndex < m_descriptorSetLayouts.size(); descriptorIndex++)
	{
		AddNoise(hashResult, (uint32)m_descriptorSetLayouts[descriptorIndex]);
	}

	//If hashResult exists in our map of registered layouts, it should be compatible with this one
	//TODO: Add support for push constant ranges
	auto mapIter = s_registeredLayouts.find(hashResult);

	if (mapIter != s_registeredLayouts.end())
	{
		m_layout = mapIter->second;
	}
	else
	{
		VkPipelineLayoutCreateInfo layoutCreateInfo;
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutCreateInfo.pNext = nullptr;
		layoutCreateInfo.flags = 0;
		layoutCreateInfo.pushConstantRangeCount = 0;
		layoutCreateInfo.pPushConstantRanges = nullptr;
		layoutCreateInfo.setLayoutCount = m_descriptorSetLayouts.size();
		layoutCreateInfo.pSetLayouts = m_descriptorSetLayouts.data();

		HLogicalDevice* device = HManager::GetLogicalDevice();

		H_ASSERT(vkCreatePipelineLayout(*device, &layoutCreateInfo, nullptr, &m_layout), "Could not create pipeline layout\n");

		s_registeredLayouts.insert(std::make_pair(hashResult, m_layout));
	}
}
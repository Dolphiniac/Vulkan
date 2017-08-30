#include "Quantum/Hephaestus/MaterialState.h"
#include "Quantum/Hephaestus/Pipeline.h"
#include "Quantum/Hephaestus/BufferDescriptor.h"
#include "Quantum/Hephaestus/Manager.h"
#include "Quantum/Hephaestus/LogicalDevice.h"
#include "Quantum/Hephaestus/CommandBuffer.h"
#include "Quantum/Core/String.h"
#include "Quantum/Hephaestus/PhysicalDevice.h"

#include <vulkan.h>


//-----------------------------------------------------------------------------------------------
STATIC HephDescriptorPool HMaterialState::s_pool = H_NULL_HANDLE;


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HMaterialState::HMaterialState(HPipeline* pipeline)
	: m_pipeline(pipeline)
{
	AllocateDescriptorSets();
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
void HMaterialState::Bind(HCommandBuffer* commandBuffer)
{
	commandBuffer->BindGraphicsPipeline(*m_pipeline);
	commandBuffer->BindGraphicsDescriptorSets(&m_descriptorSets[0], m_pipeline->m_layout, m_descriptorSets.size());
}


//-----------------------------------------------------------------------------------------------
void HMaterialState::AllocateDescriptorSets()
{
	uint32 numDescSetsTotal = m_pipeline->m_descriptorSetLayouts.size();
	m_descriptorSets.resize(numDescSetsTotal);

	HUniform::GetDescriptorSetAndLayout(nullptr, &m_descriptorSets[0]);


	if (numDescSetsTotal > 1)
	{
		if (s_pool == H_NULL_HANDLE)
		{
			InitializeDescriptorPool();
		}

		VkDescriptorSetAllocateInfo descSetAllocInfo;
		descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descSetAllocInfo.pNext = nullptr;
		descSetAllocInfo.descriptorPool = s_pool;
		descSetAllocInfo.descriptorSetCount = numDescSetsTotal - 1;	//All but the global one
		descSetAllocInfo.pSetLayouts = &m_pipeline->m_descriptorSetLayouts[1];

		HLogicalDevice* device = HManager::GetLogicalDevice();

		H_ASSERT(vkAllocateDescriptorSets(*device, &descSetAllocInfo, &m_descriptorSets[1]), "Could not allocate descriptor sets\n");
	}
}


//-----------------------------------------------------------------------------------------------
STATIC void HMaterialState::InitializeDescriptorPool()
{
	VkDescriptorPoolCreateInfo poolCreateInfo;
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.pNext = nullptr;
	poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolCreateInfo.maxSets = 256;
	poolCreateInfo.poolSizeCount = 3;

	VkDescriptorPoolSize poolSizes[3];
	poolSizes[0].descriptorCount = 256;
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = 256;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = 256;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	poolCreateInfo.pPoolSizes = poolSizes;

	HLogicalDevice* device = HManager::GetLogicalDevice();

	H_ASSERT(vkCreateDescriptorPool(*device, &poolCreateInfo, nullptr, &s_pool), "Could not create descriptor pool\n");
}


//-----------------------------------------------------------------------------------------------
uint32 HMaterialState::FindBindPointFor(uint32 nameHash) const
{
	auto reflectionIter = m_pipeline->m_pipelineReflection.descriptors.find(nameHash);
	ASSERT_OR_DIE(reflectionIter != m_pipeline->m_pipelineReflection.descriptors.end(), "Name does not exist in reflection data\n");

	return reflectionIter->second.bindingPoint;
}


//-----------------------------------------------------------------------------------------------
void HMaterialState::BindUniformBufferData(const QuString& name, void* data, uint32 dataSize)
{
	uint32 binding = FindBindPointFor(name);

	auto memoryIter = m_inUseMemoryBlocks.find(binding);
	HLogicalDevice* device = HManager::GetLogicalDevice();
	VkDeviceMemory memory;
	if (memoryIter == m_inUseMemoryBlocks.end())
	{
		VkBufferCreateInfo bufferCreateInfo;
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = nullptr;
		bufferCreateInfo.flags = 0;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferCreateInfo.pQueueFamilyIndices = nullptr;
		bufferCreateInfo.queueFamilyIndexCount = 0;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferCreateInfo.size = dataSize;

		VkBuffer buffer;
		vkCreateBuffer(*device, &bufferCreateInfo, nullptr, &buffer);

		VkMemoryRequirements memReq;
		vkGetBufferMemoryRequirements(*device, buffer, &memReq);
		HPhysicalDevice* gpu = HManager::GetPhysicalDevice();
		VkMemoryAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = gpu->GetMemoryTypeIndex(memReq.memoryTypeBits, H_MEMORY_TYPE_HOST_READABLE);

		H_ASSERT(vkAllocateMemory(*device, &allocInfo, nullptr, &memory), "Could not allocate uniform buffer memory\n");
		H_ASSERT(vkBindBufferMemory(*device, buffer, memory, 0), "Could not back uniform buffer\n");
		VkWriteDescriptorSet writeSet;
		writeSet.dstSet = m_descriptorSets[1];
		writeSet.descriptorCount = 1;
		writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSet.pNext = nullptr;
		writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeSet.dstArrayElement = 0;
		writeSet.dstBinding = binding;
		VkDescriptorBufferInfo buffInfo;
		buffInfo.buffer = buffer;
		buffInfo.offset = 0;
		buffInfo.range = VK_WHOLE_SIZE;
		writeSet.pBufferInfo = &buffInfo;
		writeSet.pImageInfo = nullptr;
		writeSet.pTexelBufferView = nullptr;
		vkUpdateDescriptorSets(*device, 1, &writeSet, 0, nullptr);

		m_inUseMemoryBlocks.insert(std::make_pair(binding, memory));
	}
	else
	{
		memory = memoryIter->second;
	}

	void* memData;
	H_ASSERT(vkMapMemory(*device, memory, 0, VK_WHOLE_SIZE, 0, &memData), "Could not map uniform buffer data\n");
	memcpy(memData, data, dataSize);
	vkUnmapMemory(*device, memory);
}


//-----------------------------------------------------------------------------------------------
void HMaterialState::BindTexelBufferData(const QuString& name, void* data, uint32 dataSize)
{
	uint32 binding = FindBindPointFor(name);

	auto memoryIter = m_inUseMemoryBlocks.find(binding);
	HLogicalDevice* device = HManager::GetLogicalDevice();
	VkDeviceMemory memory;
	if (memoryIter == m_inUseMemoryBlocks.end())
	{
		VkBufferCreateInfo bufferCreateInfo;
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = nullptr;
		bufferCreateInfo.flags = 0;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferCreateInfo.pQueueFamilyIndices = nullptr;
		bufferCreateInfo.queueFamilyIndexCount = 0;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		bufferCreateInfo.size = dataSize;

		VkBuffer buffer;
		vkCreateBuffer(*device, &bufferCreateInfo, nullptr, &buffer);

		VkMemoryRequirements memReq;
		vkGetBufferMemoryRequirements(*device, buffer, &memReq);
		HPhysicalDevice* gpu = HManager::GetPhysicalDevice();
		VkMemoryAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = gpu->GetMemoryTypeIndex(memReq.memoryTypeBits, H_MEMORY_TYPE_HOST_READABLE);

		H_ASSERT(vkAllocateMemory(*device, &allocInfo, nullptr, &memory), "Could not allocate uniform buffer memory\n");
		H_ASSERT(vkBindBufferMemory(*device, buffer, memory, 0), "Could not back uniform buffer\n");
		VkWriteDescriptorSet writeSet;
		writeSet.dstSet = m_descriptorSets[1];
		writeSet.descriptorCount = 1;
		writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSet.pNext = nullptr;
		writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		writeSet.dstArrayElement = 0;
		writeSet.dstBinding = binding;
		VkBufferViewCreateInfo bufferViewCreateInfo;
		bufferViewCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		bufferViewCreateInfo.pNext = nullptr;
		bufferViewCreateInfo.flags = 0;
		bufferViewCreateInfo.format = VK_FORMAT_R32G32B32_SFLOAT;
		bufferViewCreateInfo.offset = 0;
		bufferViewCreateInfo.range = VK_WHOLE_SIZE;
		bufferViewCreateInfo.buffer = buffer;
		VkBufferView view;
		H_ASSERT(vkCreateBufferView(*device, &bufferViewCreateInfo, nullptr, &view), "Could not create buffer view\n");
		writeSet.pBufferInfo = nullptr;
		writeSet.pImageInfo = nullptr;
		writeSet.pTexelBufferView = &view;
		vkUpdateDescriptorSets(*device, 1, &writeSet, 0, nullptr);

		m_inUseMemoryBlocks.insert(std::make_pair(binding, memory));
	}
	else
	{
		memory = memoryIter->second;
	}

	void* memData;
	H_ASSERT(vkMapMemory(*device, memory, 0, VK_WHOLE_SIZE, 0, &memData), "Could not map uniform buffer data\n");
	memcpy(memData, data, dataSize);
	vkUnmapMemory(*device, memory);
}
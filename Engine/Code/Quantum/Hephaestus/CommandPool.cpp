#include "Quantum/Hephaestus/CommandPool.h"
#include "Quantum/Hephaestus/LogicalDevice.h"
#include "Quantum/Hephaestus/CommandBuffer.h"

#include <vulkan.h>


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HCommandPool::HCommandPool(HLogicalDevice* device, uint32 queueFamilyIndex, bool isTransient)
	: m_isTransient(isTransient)
	, m_inUseBuffers(0)
	, m_device(device)
{
	VkCommandPoolCreateInfo commandPoolCreateInfo;
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = nullptr;
	if (m_isTransient)
	{
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	}
	else
	{
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	}
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

	H_ASSERT(vkCreateCommandPool(*device, &commandPoolCreateInfo, nullptr, &m_pool), "Could not create command pool\n");
}

HCommandPool::~HCommandPool()
{
	ASSERT_OR_DIE(m_inUseBuffers == 0, "Cannot destroy command pool that has in-use buffers\n");

	vkDestroyCommandPool(*m_device, m_pool, nullptr);
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HCommandBuffer* HCommandPool::RetrieveCommandBuffer(bool isPrimary)
{
	HCommandBuffer* result = nullptr;
	if (m_isTransient)
	{
		result = new HCommandBuffer(m_device, this, isPrimary);
	}
	else
	{
		if (isPrimary)
		{
			if (m_availablePrimaryBuffers.empty())
			{
				result = new HCommandBuffer(m_device, this, isPrimary);
			}
			else
			{
				result = m_availablePrimaryBuffers.back();
				m_availablePrimaryBuffers.pop_back();
			}
		}
		else
		{
			if (m_availableSecondaryBuffers.empty())
			{
				result = new HCommandBuffer(m_device, this, isPrimary);
			}
			else
			{
				result = m_availableSecondaryBuffers.back();
				m_availableSecondaryBuffers.pop_back();
			}
		}
	}
	m_inUseBuffers++;
	return result;
}


//-----------------------------------------------------------------------------------------------
HCommandBufferImmediate* HCommandPool::RetrieveImmediateCommandBuffer()
{
	return new HCommandBufferImmediate(m_device, this);
}
#include "Quantum/Hephaestus/Queue.h"
#include "Quantum/Hephaestus/CommandBuffer.h"
#include "Quantum/Hephaestus/LogicalDevice.h"

#include <vulkan.h>


//-----------------------------------------------------------------------------------------------
STATIC void HQueue::SetupQueue(HQueue* pOutQueue, float desiredPriority, HQueueCapabilityFlags capabilityFlags)
{
	ASSERT_OR_DIE(pOutQueue, "Must provide empty HQueue object\n");

	pOutQueue->m_priority = desiredPriority;
	pOutQueue->m_desiredCapabilities = capabilityFlags;
}


//-----------------------------------------------------------------------------------------------
HCommandBuffer* HQueue::RetrieveCommandBuffer(bool isTransient, bool isPrimary /* = true */)
{
	ASSERT_OR_DIE(IsInitialized(), "Cannot retrieve command buffer from uninitialized queue\n");
	return m_device->RetrieveCommandBuffer(isTransient, m_queueFamilyIndex, isPrimary);
}


//-----------------------------------------------------------------------------------------------
HCommandBufferImmediate* HQueue::RetrieveImmediateCommandBuffer()
{
	if (!m_immediateCommandBuffer)
	{
		m_immediateCommandBuffer = m_device->RetrieveImmediateCommandBuffer(m_queueFamilyIndex);
		m_immediateCommandBuffer->m_queue = this;
	}

	return m_immediateCommandBuffer;
}
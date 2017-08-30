#include "Quantum/Hephaestus/QueueFamily.h"
#include "Quantum/Hephaestus/CommandPool.h"


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HQueueFamily::HQueueFamily(HLogicalDevice* device, uint32 queueFamilyIndex)
	: m_queueFamilyIndex(queueFamilyIndex)
{
	m_transientPool = new HCommandPool(device, queueFamilyIndex, true);
	m_resetPool = new HCommandPool(device, queueFamilyIndex, false);
}


//-----------------------------------------------------------------------------------------------
HQueueFamily::~HQueueFamily()
{
	SAFE_DELETE(m_transientPool);
	SAFE_DELETE(m_resetPool);
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HCommandBuffer* HQueueFamily::RetrieveCommandBuffer(bool isTransient, bool isPrimary)
{
	if (isTransient)
	{
		return m_transientPool->RetrieveCommandBuffer(isPrimary);
	}
	else
	{
		return m_resetPool->RetrieveCommandBuffer(isPrimary);
	}
}


//-----------------------------------------------------------------------------------------------
HCommandBufferImmediate* HQueueFamily::RetrieveImmediateCommandBuffer()
{
	return m_resetPool->RetrieveImmediateCommandBuffer();
}
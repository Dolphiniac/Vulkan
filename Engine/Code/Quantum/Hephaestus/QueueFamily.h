#pragma once

#include "Quantum/Hephaestus/Declarations.h"


//-----------------------------------------------------------------------------------------------
class HQueueFamily
{
	friend class HLogicalDevice;

private:
	HQueueFamily(class HLogicalDevice* device, uint32 queueFamilyIndex);
	~HQueueFamily();

private:
	class HCommandBuffer* RetrieveCommandBuffer(bool isTransient, bool isPrimary);
	class HCommandBufferImmediate* RetrieveImmediateCommandBuffer();

private:
	class HCommandPool* m_transientPool;
	class HCommandPool* m_resetPool;
	uint32 m_queueFamilyIndex = H_INVALID;
};
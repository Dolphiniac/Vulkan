#pragma once

#include "Quantum/Hephaestus/Declarations.h"

#include <vector>


//-----------------------------------------------------------------------------------------------
class HCommandPool
{
	friend class HQueueFamily;

private:
	HCommandPool(class HLogicalDevice* device, uint32 queueFamilyIndex, bool isTransient);
	~HCommandPool();

private:
	class HCommandBuffer* RetrieveCommandBuffer(bool isPrimary);
	class HCommandBufferImmediate* RetrieveImmediateCommandBuffer();

private:
	bool m_isTransient = false;
	std::vector<class HCommandBuffer*> m_availablePrimaryBuffers;
	std::vector<class HCommandBuffer*> m_availableSecondaryBuffers;
	uint32 m_inUseBuffers = H_INVALID;
	class HLogicalDevice* m_device = nullptr;

public:
	HephCommandPool m_pool = H_NULL_HANDLE;
};
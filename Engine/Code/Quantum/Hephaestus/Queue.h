#pragma once

#include "Quantum/Hephaestus/Declarations.h"


//-----------------------------------------------------------------------------------------------
class HQueue
{
	friend class HLogicalDevice;

public:
	static void SetupQueue(HQueue* pOutQueue, float desiredPriority, HQueueCapabilityFlags capabilityFlags);
	class HCommandBuffer* RetrieveCommandBuffer(bool isTransient, bool isPrimary = true);
	class HCommandBufferImmediate* RetrieveImmediateCommandBuffer();
	operator HephQueue() const { return m_queue; }

private:
	bool IsInitialized() const { return m_queueIndex != H_INVALID; }

private:
	float m_priority;
	HQueueCapabilityFlags m_desiredCapabilities;
	uint32 m_queueFamilyIndex = H_INVALID;
	uint32 m_queueIndex = H_INVALID;
	class HLogicalDevice* m_device = nullptr;
	class HCommandBufferImmediate* m_immediateCommandBuffer = nullptr;

private:
	HephQueue m_queue = nullptr;
};
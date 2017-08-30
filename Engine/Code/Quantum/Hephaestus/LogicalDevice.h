#pragma once

#include "Quantum/Hephaestus/Declarations.h"


//-----------------------------------------------------------------------------------------------
class HLogicalDevice
{
	friend class HManager;
	friend class HQueue;

public:
	operator HephDevice() const { return m_device; }

private:
	HLogicalDevice(class HPhysicalDevice* gpu, bool enableValidation, const HQueueTransmitter& queueCreationProps);
	~HLogicalDevice();

private:
	bool GetQueueCreateInfos(class HPhysicalDevice* gpu, HephDeviceQueueCreateInfo* ppOutQueueCreateInfos, const HQueueTransmitter& queueTransmitter);
	bool GetQueueFamilyIndex(class HPhysicalDevice* gpu, HQueueCapabilityFlags desiredCapabilities, uint32& outFamilyIndex);
	void RetrieveIndexIntoFamily(class HPhysicalDevice* gpu, class HQueue** ppQueueInitializers, uint32 currentQueueIndex);
	void FillQueues();
	class HCommandBuffer* RetrieveCommandBuffer(bool isTransient, uint32 queueFamilyIndex, bool isPrimary);
	class HCommandBufferImmediate* RetrieveImmediateCommandBuffer(uint32 queueFamilyIndex);

public:
	class HQueue** m_queues = nullptr;
	class HQueueFamily** m_queueFamilies = nullptr;
	uint32 m_numQueues = H_INVALID;
	uint32 m_numQueueFamilies = H_INVALID;
	
private:
	HephDevice m_device = nullptr;
};
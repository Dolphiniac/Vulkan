#pragma once

#include "Quantum/Hephaestus/Declarations.h"


//-----------------------------------------------------------------------------------------------
class HPhysicalDevice
{
	friend class HManager;

public:
	uint32 GetMemoryTypeIndex(uint32 memoryTypeBits, EMemoryType memoryType);
	operator HephPhysicalDevice() const { return m_device; }

private:
	HPhysicalDevice(class HInstance* instance, bool integrated);
	~HPhysicalDevice();

public:
	HephPhysicalDevice m_device = nullptr;
	HephPhysicalDeviceFeatures m_features = nullptr;
	HephQueueFamilyProperties m_queueFamilyProps = nullptr;
	HephPhysicalDeviceMemoryProperties m_memoryProps = nullptr;
	uint32 m_numQueueFamilies = H_INVALID;
};
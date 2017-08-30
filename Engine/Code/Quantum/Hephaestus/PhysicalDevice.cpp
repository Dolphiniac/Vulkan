#include "Quantum/Hephaestus/PhysicalDevice.h"
#include "Quantum/Hephaestus/Instance.h"

#include <vulkan.h>


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------
#include <vector>

//-----------------------------------------------------------------------------------------------
HPhysicalDevice::HPhysicalDevice(HInstance* instance, bool integrated)
{
	uint32 numPhysicalDevices = H_INVALID;
	H_ASSERT(vkEnumeratePhysicalDevices(instance->m_instance, &numPhysicalDevices, nullptr), 
		"Could not get physical device count\n");

	HephPhysicalDevice* gpus = new HephPhysicalDevice[numPhysicalDevices];

	H_ASSERT(vkEnumeratePhysicalDevices(instance->m_instance, &numPhysicalDevices, gpus),
		"Could not get physical devices\n");

	//Walk physical devices and find requested gpu
	for (uint32 deviceIndex = 0; deviceIndex < numPhysicalDevices; deviceIndex)
	{
		VkPhysicalDeviceProperties gpuProperties;
		vkGetPhysicalDeviceProperties(gpus[deviceIndex], &gpuProperties);
		
		if (integrated)
		{
			if (gpuProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			{
				m_device = gpus[deviceIndex];
				break;
			}
		}
		else
		{
			if (gpuProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				m_device = gpus[deviceIndex];
				break;
			}
		}
	}

	ASSERT_OR_DIE(m_device, "Could not get requested device\n");

	SAFE_DELETE_ARRAY(gpus);

	//Now we want to store information about this gpu
	m_features = new VkPhysicalDeviceFeatures();

	//This enables ALL available features of the GPU.  We may want to optimize out things we don't need later
	vkGetPhysicalDeviceFeatures(m_device, m_features);

	vkGetPhysicalDeviceQueueFamilyProperties(m_device, &m_numQueueFamilies, nullptr);

	ASSERT_OR_DIE(m_numQueueFamilies > 0 && m_numQueueFamilies != H_INVALID, "Could not allocate queue family properties\n");

	m_queueFamilyProps = new VkQueueFamilyProperties[m_numQueueFamilies];
	vkGetPhysicalDeviceQueueFamilyProperties(m_device, &m_numQueueFamilies, m_queueFamilyProps);
	uint32 numLayerProps;
	vkEnumerateDeviceExtensionProperties(m_device, nullptr, &numLayerProps, nullptr);
	std::vector<VkExtensionProperties> props;
	props.resize(numLayerProps);
	vkEnumerateDeviceExtensionProperties(m_device, nullptr, &numLayerProps, &props[0]);
	m_memoryProps = new VkPhysicalDeviceMemoryProperties();
	vkGetPhysicalDeviceMemoryProperties(m_device, m_memoryProps);
}


//-----------------------------------------------------------------------------------------------
HPhysicalDevice::~HPhysicalDevice()
{
	SAFE_DELETE(m_memoryProps);
	SAFE_DELETE(m_features);
	SAFE_DELETE_ARRAY(m_queueFamilyProps);
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
uint32 HPhysicalDevice::GetMemoryTypeIndex(uint32 memoryTypeBits, EMemoryType memoryType)
{
	uint32 desiredProperties = H_INVALID;

	switch (memoryType)
	{
	case H_MEMORY_TYPE_HOST_READABLE:
		desiredProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		break;

	case H_MEMORY_TYPE_DEVICE_LOCAL:
		desiredProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		break;
	}

	for (uint32 memoryTypeIndex = 0; memoryTypeIndex < m_memoryProps->memoryTypeCount; memoryTypeIndex++)
	{
		if ((memoryTypeBits & (1 << memoryTypeIndex)) && ((m_memoryProps->memoryTypes[memoryTypeIndex].propertyFlags & desiredProperties) == desiredProperties))
		{
			//Checking first if the memory type is compatible, and then if this memory type has the properties we want
			return memoryTypeIndex;
		}
	}

	ERROR_AND_DIE("Could not find memory type index\n");
}
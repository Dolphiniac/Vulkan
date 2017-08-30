#include "Quantum/Hephaestus/LogicalDevice.h"
#include "Quantum/Hephaestus/PhysicalDevice.h"
#include "Quantum/Hephaestus/Queue.h"
#include "Quantum/Hephaestus/QueueFamily.h"
#include "Quantum/Core/String.h"

#include <vulkan.h>


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HLogicalDevice::HLogicalDevice(HPhysicalDevice* gpu, bool enableValidation, const HQueueTransmitter& queueTransmitter)
{
	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;

	std::vector<const char*> extensions;
	const char* swapchainExtension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	extensions.push_back(swapchainExtension);	//Used to allow swapchains (necessary for rendering)
	deviceCreateInfo.enabledExtensionCount = extensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = &extensions[0];

	//Validation makes Vulkan spit out meaningful errors
	if (enableValidation)
	{
		const char* validationName = H_VALIDATION_LAYER_NAME;
		deviceCreateInfo.enabledLayerCount = 1;
		deviceCreateInfo.ppEnabledLayerNames = &validationName;
	}
	else
	{
		deviceCreateInfo.enabledLayerCount = 0;
		deviceCreateInfo.ppEnabledLayerNames = nullptr;
	}

	deviceCreateInfo.pEnabledFeatures = gpu->m_features;

	HephDeviceQueueCreateInfo queueCreateInfos = nullptr;
	ASSERT_OR_DIE(GetQueueCreateInfos(gpu, &queueCreateInfos, queueTransmitter), "Could not retrieve queue create infos from properties given\n");

	deviceCreateInfo.queueCreateInfoCount = queueTransmitter.numQueueInitializers;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;

	H_ASSERT(vkCreateDevice(gpu->m_device, &deviceCreateInfo, nullptr, &m_device), "Could not create logical device\n");

	FillQueues();

	SAFE_DELETE_ARRAY(queueCreateInfos);
}


//-----------------------------------------------------------------------------------------------
HLogicalDevice::~HLogicalDevice()
{
	vkDestroyDevice(m_device, nullptr);

	SAFE_DELETE_PTR_ARRAY(m_queues, m_numQueues);
	SAFE_DELETE_PTR_ARRAY(m_queueFamilies, m_numQueueFamilies);
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
bool HLogicalDevice::GetQueueCreateInfos(HPhysicalDevice* gpu, HephDeviceQueueCreateInfo* ppOutQueueCreateInfos, const HQueueTransmitter& queueTransmitter)
{
	ASSERT_OR_DIE(queueTransmitter.numQueueInitializers > 0 && queueTransmitter.numQueueInitializers != H_INVALID,
		"Must pass in valid number of uninitialized HQueues\n");
	ASSERT_OR_DIE(queueTransmitter.ppQueueInitializers, "Must pass in valid array of uninitialized HQueues\n");

	*ppOutQueueCreateInfos = new VkDeviceQueueCreateInfo[queueTransmitter.numQueueInitializers];
	m_numQueues = queueTransmitter.numQueueInitializers;
	m_queues = new HQueue*[m_numQueues];

	for (uint32 queueIndex = 0; queueIndex < queueTransmitter.numQueueInitializers; queueIndex++)
	{
		HQueue* queue = queueTransmitter.ppQueueInitializers[queueIndex];

		QuString invalidHQueue = QuString::F("Invalid HQueue at index %u", queueIndex);
		ASSERT_OR_DIE(queue, invalidHQueue.GetRaw());

		//Creating a heap-allocated copy so we control the memory
		m_queues[queueIndex] = new HQueue(*queue);
		queue = m_queues[queueIndex];

		HephDeviceQueueCreateInfo currentQueueCreateInfo = ppOutQueueCreateInfos[queueIndex];
		currentQueueCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		currentQueueCreateInfo->pNext = nullptr;
		currentQueueCreateInfo->flags = 0;
		currentQueueCreateInfo->queueCount = 1;
		currentQueueCreateInfo->pQueuePriorities = &queue->m_priority;
		bool gotFamilyIndex = GetQueueFamilyIndex(gpu, queue->m_desiredCapabilities, queue->m_queueFamilyIndex);
		if (!gotFamilyIndex)
		{
			DebuggerPrintf("Could not retrieve queue family index for capability mask %u\n", queue->m_desiredCapabilities);
			return false;
		}

		currentQueueCreateInfo->queueFamilyIndex = queue->m_queueFamilyIndex;

		//Now that we've filled out the device queue create info, we need to find the index into our family, so we can
		//retrieve the device queue that corresponds to this HQueue
		RetrieveIndexIntoFamily(gpu, m_queues, queueIndex);
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool HLogicalDevice::GetQueueFamilyIndex(HPhysicalDevice* gpu, HQueueCapabilityFlags desiredCapabilities, uint32& outFamilyIndex)
{
	uint32 desiredFlags = 0;
	if (desiredCapabilities & H_QUEUE_CAPABILITY_GRAPHICS_BIT)
	{
		desiredFlags |= VK_QUEUE_GRAPHICS_BIT;
	}
	if (desiredCapabilities & H_QUEUE_CAPABILITY_COMPUTE_BIT)
	{
		desiredFlags |= VK_QUEUE_COMPUTE_BIT;
	}
	if (desiredCapabilities & H_QUEUE_CAPABILITY_TRANSFER_BIT)
	{
		desiredFlags |= VK_QUEUE_TRANSFER_BIT;
	}
	if (desiredCapabilities & H_QUEUE_CAPABILITY_SPARSE_BINDING_BIT)
	{
		desiredFlags |= VK_QUEUE_SPARSE_BINDING_BIT;
	}

	for (uint32 queueFamilyIndex = 0; queueFamilyIndex < gpu->m_numQueueFamilies; queueFamilyIndex++)
	{
		HephQueueFamilyProperties properties = &gpu->m_queueFamilyProps[queueFamilyIndex];
		if ((properties->queueFlags & desiredFlags) == desiredFlags)
		{
			outFamilyIndex = queueFamilyIndex;
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void HLogicalDevice::RetrieveIndexIntoFamily(HPhysicalDevice* gpu, class HQueue** ppQueueInitializers, uint32 currentQueueIndex)
{
	HQueue* thisQueue = ppQueueInitializers[currentQueueIndex];

	if (currentQueueIndex == 0)
	{
		thisQueue->m_queueIndex = 0;
	}
	else
	{
		for (uint32 queueIndex = currentQueueIndex - 1; queueIndex >= 0; queueIndex--)
		{
			HQueue* currentQueue = ppQueueInitializers[queueIndex];

			if (currentQueue->m_queueFamilyIndex == thisQueue->m_queueFamilyIndex)
			{
				thisQueue->m_queueIndex = currentQueue->m_queueIndex + 1;
				break;
			}
		}
		if (thisQueue->m_queueIndex == H_INVALID)
		{
			thisQueue->m_queueIndex = 0;
		}
	}

	QuString queueOverloadMessage = QuString::F("Cannot create all queues.  Queue %u of family %u exceeds device capabilities\n",
		thisQueue->m_queueIndex, thisQueue->m_queueFamilyIndex);
	ASSERT_OR_DIE(thisQueue->m_queueIndex < gpu->m_queueFamilyProps[thisQueue->m_queueFamilyIndex].queueCount, queueOverloadMessage.GetRaw());
}


//-----------------------------------------------------------------------------------------------
void HLogicalDevice::FillQueues()
{
	uint32 maxQueueFamilyIndex = H_INVALID;
	//Here, since each HQueue now knows both its family index and queue index, it can store the device queue handle
	for (uint32 queueIndex = 0; queueIndex < m_numQueues; queueIndex++)
	{
		HQueue* currentQueue = m_queues[queueIndex];

		vkGetDeviceQueue(m_device, currentQueue->m_queueFamilyIndex, currentQueue->m_queueIndex, &currentQueue->m_queue);

		if (maxQueueFamilyIndex == H_INVALID || maxQueueFamilyIndex < currentQueue->m_queueFamilyIndex)
		{
			maxQueueFamilyIndex = currentQueue->m_queueFamilyIndex;
		}

		currentQueue->m_device = this;
	}

	//Now we create the families array, and then create command pools for each valid one
	m_numQueueFamilies = maxQueueFamilyIndex + 1;
	m_queueFamilies = new HQueueFamily*[m_numQueueFamilies];
	memset(m_queueFamilies, 0, sizeof(HQueueFamily*) * m_numQueueFamilies);

	for (uint32 queueIndex = 0; queueIndex < m_numQueues; queueIndex++)
	{
		HQueue* currentQueue = m_queues[queueIndex];

		HQueueFamily* queueFamily = m_queueFamilies[currentQueue->m_queueFamilyIndex];

		if (!queueFamily)
		{
			queueFamily = new HQueueFamily(this, currentQueue->m_queueFamilyIndex);
			m_queueFamilies[currentQueue->m_queueFamilyIndex] = queueFamily;
		}
	}
}


//-----------------------------------------------------------------------------------------------
HCommandBuffer* HLogicalDevice::RetrieveCommandBuffer(bool isTransient, uint32 queueFamilyIndex, bool isPrimary)
{
	return m_queueFamilies[queueFamilyIndex]->RetrieveCommandBuffer(isTransient, isPrimary);
}


//-----------------------------------------------------------------------------------------------
HCommandBufferImmediate* HLogicalDevice::RetrieveImmediateCommandBuffer(uint32 queueFamilyIndex)
{
	return m_queueFamilies[queueFamilyIndex]->RetrieveImmediateCommandBuffer();
}
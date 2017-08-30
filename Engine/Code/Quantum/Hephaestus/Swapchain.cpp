#include "Quantum/Hephaestus/Swapchain.h"
#include "Quantum/Hephaestus/PhysicalDevice.h"
#include "Quantum/Hephaestus/LogicalDevice.h"
#include "Quantum/Hephaestus/Surface.h"

#include <vulkan.h>


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HSwapchain::HSwapchain(HPhysicalDevice* gpu, HLogicalDevice* device, HSurface* surface, EPresentationMode presentationMode)
{
	VkSwapchainCreateInfoKHR swapchainCreateInfo;
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = surface->m_surface;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE; //If window is resized or similar, we'll need to transfer to another swapchain

	//Again, not sure of the point of this.  Seems to me that a swapchain's purpose is to allow screen rendering
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	//Sharing the swapchain image?  Maybe useful if splitting rendering tasks, but not good for me right now
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;

	swapchainCreateInfo.clipped = VK_TRUE; //Why not?
	
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	H_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu->m_device, surface->m_surface, &surfaceCapabilities), "Could not get surface capabilities\n");

	uint32 surfaceFormatCount;
	H_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu->m_device, surface->m_surface, &surfaceFormatCount, nullptr), "Could not get number of surface formats\n");

	VkSurfaceFormatKHR* defaultFormats = new VkSurfaceFormatKHR[surfaceFormatCount];
	H_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu->m_device, surface->m_surface, &surfaceFormatCount, defaultFormats), "Could not get surface formats\n");

	uint32 presentModeCount;
	H_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu->m_device, surface->m_surface, &presentModeCount, nullptr), "Could not retrieve number of presentation modes\n");

	VkPresentModeKHR* presentModes = new VkPresentModeKHR[presentModeCount];
	H_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu->m_device, surface->m_surface, &presentModeCount, presentModes), "Could not retrieve present modes\n");

	SetPresentModeAndImageCount(&swapchainCreateInfo, presentationMode, presentModes, presentModeCount, &surfaceCapabilities);

	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; //Not sure how to automate a more reasonable transform, so just setting this to default
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //Again, not sure how to automate.  Right now, only one type is allowed
	swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent; //I think that this can be uninitialized, but I don't expect it right now

	//I don't really see the usage of changing the formats, so I'll just use a default
	swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	swapchainCreateInfo.imageColorSpace = defaultFormats->colorSpace;

	H_ASSERT(vkCreateSwapchainKHR(*device, &swapchainCreateInfo, nullptr, &m_swapchain), "Could not create swapchain\n");

	SAFE_DELETE_ARRAY(presentModes);
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
void HSwapchain::SetPresentModeAndImageCount(HephSwapchainCreateInfoKHR swapchainCreateInfo, EPresentationMode desiredPresentationMode, HephPresentModeKHR* presentModes, 
	uint32 presentModeCount, HephSurfaceCapabilitiesKHR surfaceCapabilities)
{
	switch (desiredPresentationMode)
	{
	case H_PRESENTATION_MODE_DOUBLE_BUFFER:
		swapchainCreateInfo->minImageCount = 2;
		swapchainCreateInfo->presentMode = VK_PRESENT_MODE_FIFO_KHR;
		break;
	case H_PRESENTATION_MODE_IMMEDIATE:
		swapchainCreateInfo->minImageCount = 2;
		if (IsSupported(VK_PRESENT_MODE_IMMEDIATE_KHR, 2, surfaceCapabilities, presentModes, presentModeCount))
		{
			swapchainCreateInfo->presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
		else
		{
			swapchainCreateInfo->presentMode = VK_PRESENT_MODE_FIFO_KHR;
		}
		break;
	case H_PRESENTATION_MODE_TRIPLE_BUFFER:
		if (IsSupported(VK_PRESENT_MODE_MAILBOX_KHR, 3, surfaceCapabilities, presentModes, presentModeCount))
		{
			swapchainCreateInfo->minImageCount = 3;
			swapchainCreateInfo->presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		}
		else
		{
			swapchainCreateInfo->minImageCount = 2;
			swapchainCreateInfo->presentMode = VK_PRESENT_MODE_FIFO_KHR;
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
bool HSwapchain::IsSupported(HephPresentModeKHR presentMode, uint32 imageCount, HephSurfaceCapabilitiesKHR surfaceCapabilities, HephPresentModeKHR* presentModes, uint32 presentModeCount)
{
	if (imageCount > surfaceCapabilities->maxImageCount)
	{
		return false;
	}

	for (uint32 modeIndex = 0; modeIndex < presentModeCount; modeIndex++)
	{
		if (presentModes[modeIndex] == presentMode)
		{
			return true;
		}
	}

	return false;
}
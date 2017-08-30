#pragma once

#include "Quantum/Hephaestus/Declarations.h"


//-----------------------------------------------------------------------------------------------
class HSwapchain
{
	friend class HManager;

public:
	operator HephSwapchainKHR() const { return m_swapchain; }
	const HephSwapchainKHR& GetSwapchain() const { return m_swapchain; }

private:
	HSwapchain(class HPhysicalDevice* gpu, class HLogicalDevice* device, class HSurface* surface, EPresentationMode presentationMode);

private:
	void SetPresentModeAndImageCount(HephSwapchainCreateInfoKHR swapchainCreateInfo, EPresentationMode desiredPresentationMode, HephPresentModeKHR* presentModes, 
		uint32 presentModeCount, HephSurfaceCapabilitiesKHR surfaceCapabilities);
	bool IsSupported(HephPresentModeKHR presentMode, uint32 imageCount, HephSurfaceCapabilitiesKHR surfaceCapabilities, HephPresentModeKHR* presentModes, uint32 presentModeCount);

private:
	HephSwapchainKHR m_swapchain;
};
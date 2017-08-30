#include "Quantum/Hephaestus/Instance.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan.h>

//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HInstance::HInstance(bool enableValidation)
{
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.apiVersion = VK_API_VERSION_1_0;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pApplicationName = "Unused";
	appInfo.pEngineName = "Hephaestus";
	
	VkInstanceCreateInfo instanceCreateInfo;
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	//If we enable validation, Vulkan will give us meaningful errors
	if (enableValidation)
	{
		const char* validationLayer = H_VALIDATION_LAYER_NAME;
		instanceCreateInfo.enabledLayerCount = 1;
		instanceCreateInfo.ppEnabledLayerNames = &validationLayer;
	}
	else
	{
		instanceCreateInfo.enabledLayerCount = 0;
		instanceCreateInfo.ppEnabledLayerNames = nullptr;
	}
	
	//Necessary extensions for win32 surfaces
	const char* extensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	instanceCreateInfo.enabledExtensionCount = ARRAY_LENGTH(extensions);
	instanceCreateInfo.ppEnabledExtensionNames = extensions;

	H_ASSERT(vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance), "Could not start Vulkan instance");
}


//-----------------------------------------------------------------------------------------------
HInstance::~HInstance()
{
	vkDestroyInstance(m_instance, nullptr);
}



//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------
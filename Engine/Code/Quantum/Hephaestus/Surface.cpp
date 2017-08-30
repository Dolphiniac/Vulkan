#include "Quantum/Hephaestus/Surface.h"
#include "Quantum/Hephaestus/Instance.h"
#include "Quantum/Hephaestus/PhysicalDevice.h"
#include "Quantum/Hephaestus/Manager.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan.h>


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HSurface::HSurface(HInstance* instance, HephHINSTANCE applicationHandle, HephHWND windowHandle)
{
 	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
 	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
 	surfaceCreateInfo.pNext = nullptr;
 	surfaceCreateInfo.flags = 0;
 	surfaceCreateInfo.hinstance = applicationHandle;
 	surfaceCreateInfo.hwnd = windowHandle;
 
 	H_ASSERT(vkCreateWin32SurfaceKHR(instance->m_instance, &surfaceCreateInfo, nullptr, &m_surface), "Could not create Win32 surface\n");
 
	VkBool32 isSupported;
 	vkGetPhysicalDeviceSurfaceSupportKHR(HManager::GetPhysicalDevice()->m_device, 0, m_surface, &isSupported); //Suppressing a warning, but maybe should revisit
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------
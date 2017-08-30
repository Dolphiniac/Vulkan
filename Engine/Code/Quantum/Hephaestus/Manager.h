#pragma once

#include "Quantum/Hephaestus/Declarations.h"
#include "Quantum/Hephaestus/Queue.h"

#include <map>


//-----------------------------------------------------------------------------------------------
class HManager
{
	friend class HShader;
	friend class HPipelineGenerator;
	friend class HUniform;
	friend class HPipeline;
	friend class HSurface;

public:
	static void CreateInstance(bool enableValidation = true);
	static void RetrieveAndSetGPU(bool integratedCard = false);
	
	//The queue transmitter will not return your queues by pointer.  It will make copies so that
	//the logical device can handle the memory
	static void CreateLogicalDevice(const HQueueTransmitter& queueTransmitter, bool enableValidation = true);

	//Helper function for creating a single-threaded rendering device.
	//Place HQueue** to retrieve the basic HQueue* at this time, or nullptr to retrieve the queue later
	static void CreateLogicalDeviceSimple(class HQueue** ppOutQueue = nullptr, bool enableValidation = true);
	static class HQueue* RetrieveQueue(uint32 index = 0);
	static void RetrieveQueues(class HQueue**& ppOutQueues, uint32& outNumQueues);

	static void InitializeWin32Surface(HephHINSTANCE applicationHandle, HephHWND windowHandle);

	static void CreateSwapchain(uint32 width, uint32 height, EPresentationMode presentationMode = H_PRESENTATION_MODE_TRIPLE_BUFFER);
	static void SetRenderPass(class HRenderPass* renderPass) { s_currentRenderPass = renderPass; }

	static void Deinitialize();
	static class HPipelineGenerator* GetPipelineGenerator() { return s_pipelineGenerator; }
	static class HDescriptorSetLayoutGenerator* GetDescriptorSetLayoutGenerator() { return s_descriptorSetLayoutGenerator; }
	static class HSwapchain* GetSwapchain() { return s_swapchain; }
	static class HLogicalDevice* GetLogicalDevice() { return s_logicalDevice; }
	static class HPhysicalDevice* GetPhysicalDevice() { return s_physicalDevice; }
	static class HQueue* GetGraphicsQueue() { return s_graphicsQueue; }	//This might conflict with RetrieveQueue, may want to go more towards grabbing queues by type exclusively
	static class HQueue* GetTransferQueue() { return s_transferQueue; }
	static class HRenderPass* GetCurrentRenderPass() { return s_currentRenderPass; }
	static class HRenderPass* GetRenderPassByName(uint32 nameHash);
	static class HRenderPass* GetRenderPassByName(const class QuString& name);
	static void RegisterRenderPass(uint32 nameHash, class HRenderPass* renderPass);
	static uint32 GetWindowWidth() { return s_swapchainWidth; }
	static uint32 GetWindowHeight() { return s_swapchainHeight; }

	static void LinkRenderPassesBySemaphore(uint32 fromPass, uint32 toPass, EPipelineStage pipelineStage);

private:

private:
	static class HInstance* s_instance;
	static class HPhysicalDevice* s_physicalDevice;
	static class HLogicalDevice* s_logicalDevice;
	static class HSurface* s_surface;
	static class HSwapchain* s_swapchain;
	static class HPipelineGenerator* s_pipelineGenerator;
	static class HDescriptorSetLayoutGenerator* s_descriptorSetLayoutGenerator;
	static class HQueue* s_graphicsQueue;
	static class HQueue* s_transferQueue;
	static class HRenderPass* s_currentRenderPass;
	static std::map<uint32, class HRenderPass*> s_renderPasses;
	static uint32 s_swapchainWidth;
	static uint32 s_swapchainHeight;
};
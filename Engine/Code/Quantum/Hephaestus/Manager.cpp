#include "Quantum/Hephaestus/Manager.h"
#include "Quantum/Hephaestus/Instance.h"
#include "Quantum/Hephaestus/PhysicalDevice.h"
#include "Quantum/Hephaestus/LogicalDevice.h"
#include "Quantum/Hephaestus/Queue.h"
#include "Quantum/Hephaestus/Surface.h"
#include "Quantum/Hephaestus/Swapchain.h"
#include "Quantum/Hephaestus/Shader.h"
#include "Quantum/Hephaestus/VertexType.h"
#include "Quantum/Hephaestus/PipelineGenerator.h"
#include "Quantum/Hephaestus/DescriptorSetLayoutGenerator.h"
#include "Quantum/Hephaestus/RenderPass.h"
#include "Quantum/Core/String.h"

#include <vulkan.h>

#pragma comment (lib, "vulkan-1")


//-----------------------------------------------------------------------------------------------
STATIC HInstance* HManager::s_instance = nullptr;
STATIC HPhysicalDevice* HManager::s_physicalDevice = nullptr;
STATIC HLogicalDevice* HManager::s_logicalDevice = nullptr;
STATIC HSurface* HManager::s_surface = nullptr;
STATIC HSwapchain* HManager::s_swapchain = nullptr;
STATIC HPipelineGenerator* HManager::s_pipelineGenerator = nullptr;
STATIC HDescriptorSetLayoutGenerator* HManager::s_descriptorSetLayoutGenerator = nullptr;
STATIC HQueue* HManager::s_graphicsQueue = nullptr;
STATIC HQueue* HManager::s_transferQueue = nullptr;
STATIC HRenderPass* HManager::s_currentRenderPass = nullptr;
STATIC uint32 HManager::s_swapchainWidth = 0;
STATIC uint32 HManager::s_swapchainHeight = 0;
STATIC std::map<uint32, HRenderPass*> HManager::s_renderPasses;


//-----------------------------------------------------------------------------------------------
STATIC void HManager::CreateInstance(bool enableValidation /* = true */)
{
	ASSERT_OR_DIE(!s_instance, "Cannot have pre-existing Vulkan instance\n");
	s_instance = new HInstance(enableValidation);
}


//-----------------------------------------------------------------------------------------------
STATIC void HManager::RetrieveAndSetGPU(bool integratedCard /* = false */)
{
	ASSERT_OR_DIE(!s_physicalDevice, "GPU already retrieved\n");
	ASSERT_OR_DIE(s_instance, "Must create Hephaestus instance first\n");
	s_physicalDevice = new HPhysicalDevice(s_instance, integratedCard);
}


//-----------------------------------------------------------------------------------------------
STATIC void HManager::CreateLogicalDevice(const HQueueTransmitter& queueTransmitter, bool enableValidation /* = true */)
{
	ASSERT_OR_DIE(!s_logicalDevice, "Cannot have pre-existing logical device\n");
	ASSERT_OR_DIE(s_physicalDevice, "Must retrieve GPU first\n");
	s_logicalDevice = new HLogicalDevice(s_physicalDevice, enableValidation, queueTransmitter);


	s_descriptorSetLayoutGenerator = new HDescriptorSetLayoutGenerator();

	HVertexType::InitializeTypes();
}


//-----------------------------------------------------------------------------------------------
STATIC void HManager::CreateLogicalDeviceSimple(HQueue** ppOutQueue /* = nullptr */, bool enableValidation /* = true */)
{
	HQueue queue;
	HQueue* queuePtr = &queue;
	HQueue::SetupQueue(queuePtr, 1.f, H_QUEUE_CAPABILITY_ALL_BUT_SPARSE);
	
	HQueueTransmitter transmitter;
	transmitter.numQueueInitializers = 1;
	transmitter.ppQueueInitializers = &queuePtr;

	CreateLogicalDevice(transmitter, enableValidation);

	if (ppOutQueue)
	{
		s_graphicsQueue = RetrieveQueue();
		s_transferQueue = s_graphicsQueue;
		*ppOutQueue = RetrieveQueue();
	}
}


//-----------------------------------------------------------------------------------------------
STATIC HQueue* HManager::RetrieveQueue(uint32 index /* = 0 */)
{
	ASSERT_OR_DIE(s_logicalDevice, "Cannot retrieve queue from logical device that does not exist\n");
	ASSERT_OR_DIE(index < s_logicalDevice->m_numQueues, "Cannot retrieve queue at this index\n");
	return s_logicalDevice->m_queues[index];
}


//-----------------------------------------------------------------------------------------------
STATIC void HManager::RetrieveQueues(HQueue**& ppOutQueues, uint32& outNumQueues)
{
	ASSERT_OR_DIE(s_logicalDevice, "Cannot retrieve queues from logical device that does not exist\n");
	ppOutQueues = s_logicalDevice->m_queues;
	outNumQueues = s_logicalDevice->m_numQueues;
}


//-----------------------------------------------------------------------------------------------
STATIC void HManager::InitializeWin32Surface(HephHINSTANCE applicationHandle, HephHWND windowHandle)
{
	ASSERT_OR_DIE(!s_surface, "Cannot create multiple surfaces\n");
	ASSERT_OR_DIE(s_instance, "Cannot create surface without existing instance\n");

	s_surface = new HSurface(s_instance, applicationHandle, windowHandle);
}


//-----------------------------------------------------------------------------------------------
STATIC void HManager::CreateSwapchain(uint32 width, uint32 height, EPresentationMode presentationMode)
{
	ASSERT_OR_DIE(!s_swapchain, "Cannot create multiple swapchains at this time\n");
	ASSERT_OR_DIE(s_logicalDevice, "Must have logical device initialized before creating a swapchain\n");
	ASSERT_OR_DIE(s_surface, "Must have surface initialized before creating a swapchain\n");
	s_swapchain = new HSwapchain(s_physicalDevice, s_logicalDevice, s_surface, presentationMode);
	s_swapchainWidth = width;
	s_swapchainHeight = height;
	s_pipelineGenerator = new HPipelineGenerator();
}


//-----------------------------------------------------------------------------------------------
STATIC HRenderPass* HManager::GetRenderPassByName(uint32 nameHash)
{
	auto passIter = s_renderPasses.find(nameHash);
	ASSERT_OR_DIE(passIter != s_renderPasses.end(), "Render pass does not exist or is not registered\n");

	return passIter->second;
}


//-----------------------------------------------------------------------------------------------
STATIC HRenderPass* HManager::GetRenderPassByName(const QuString& name)
{
	return GetRenderPassByName(name.GetHash());
}


//-----------------------------------------------------------------------------------------------
STATIC void HManager::RegisterRenderPass(uint32 nameHash, HRenderPass* renderPass)
{
	auto passIter = s_renderPasses.find(nameHash);
	ASSERT_OR_DIE(passIter == s_renderPasses.end(), "There is already a render pass by this name registered\n");

	s_renderPasses.insert(std::make_pair(nameHash, renderPass));
}


//-----------------------------------------------------------------------------------------------
STATIC void HManager::Deinitialize()
{
	HVertexType::DeinitializeTypes();
	HShader::Deinitialize();
	SAFE_DELETE(s_logicalDevice);
	SAFE_DELETE(s_physicalDevice);
	SAFE_DELETE(s_instance);
	SAFE_DELETE(s_pipelineGenerator);
}


//-----------------------------------------------------------------------------------------------
STATIC void HManager::LinkRenderPassesBySemaphore(uint32 fromPass, uint32 toPass, EPipelineStage pipelineStage)
{
	//Adds a link from fromPass's render complete semaphore to toPass's submission's wait list
	HRenderPass* from = GetRenderPassByName(fromPass);
	HRenderPass* to = GetRenderPassByName(toPass);

	to->m_waitSemaphores.push_back(from->GenerateSemaphore());
	to->m_submitInfo->waitSemaphoreCount = to->m_waitSemaphores.size();
	to->m_submitInfo->pWaitSemaphores = &to->m_waitSemaphores[0];
	to->m_waitDstStages.push_back(Hephaestus::TranslateStage(pipelineStage));
	to->m_submitInfo->pWaitDstStageMask = &to->m_waitDstStages[0];
}
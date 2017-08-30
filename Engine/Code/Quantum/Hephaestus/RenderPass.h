#pragma once

#include "Quantum/Hephaestus/Declarations.h"
#include "Quantum/Hephaestus/CommandBuffer.h"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct HAttachmentInfo
{
	uint32 nameHash;
	std::vector<HephImageUsageFlagBits> usageFlags;
	HephFormat format;
	uint32 width;
	uint32 height;
	uint32 depth;
	bool needsCreate;
	std::vector<HephImage> images;	//There might be multiple, if it's a swapchain attachment
	std::vector<HephImageView> views;
};


//-----------------------------------------------------------------------------------------------
struct HSubpass
{
	HephRenderPass renderPass;
	uint32 subpassIndex;
};


//-----------------------------------------------------------------------------------------------
class HRenderPass
{
	friend class HManager;

public:
	static HRenderPass* Create(class HQueue* targetQueue, const class QuString& xmlName);
	HSubpass RetrieveSubpass(uint32 nameHash);
	void Begin();
	void RegisterDrawCommands(class HCommandBuffer* commandBuffer) { m_drawCmds.push_back(*commandBuffer); }
	void Complete();
	HephCommandBufferInheritanceInfo GetInheritanceInfo() const { return m_inheritanceInfo; }
	void NextSubpass();
	HephImageView GetViewForAttachment(uint32 attachmentName);
	uint32 GetHandle() const { return m_renderPassName; }

private:
	HRenderPass(class HQueue* targetQueue, const class QuString& xmlName);
	void GetCommandBuffer();
	void InitializeSynchronizationPrimitives();
	void InitializeWorkInfos();
	void SelectFramebuffer();
	void CreateFramebuffers();

	void CreateDepthImageAndView();
	void InitializeFromXML(const QuString& xmlName);
	void ParseAttachmentsNode(const struct XMLNode& node);
	void ParseAttachmentNode(const XMLNode& node);
	void ParseSubpassesNode(const XMLNode& node);
	void ParseSubpassNode(const XMLNode& node);
	void ParseSubpassAttachmentsNode(const XMLNode& node, HephSubpassDescription subpassDesc);
	void ParseSubpassAttachmentNode(const XMLNode& node, std::vector<HephAttachmentReference_T>& outAttachmentReferences);
	HAttachmentInfo* FindAttachmentInfo(uint32 name, uint32* pOutAttachmentIndex);
	void ParseDependenciesNode(const XMLNode& node);
	void ParseDependencyNode(const XMLNode& node);
	void GetDependencyValuesFromNode(const XMLNode node, uint32* pOutSubpassIndex, uint32* pOutStageMask, uint32* pOutAccessMask);
	bool FindSubpassIndex(uint32 subpassName, uint32* pOutIndex);
	HephSemaphore GenerateSemaphore();

private:
	HephRenderPass m_renderPass = H_NULL_HANDLE;
	HephRenderPassCreateInfo m_createInfo = nullptr;
	HephRenderPassBeginInfo m_beginInfo = nullptr;
	HephCommandBufferInheritanceInfo m_inheritanceInfo = nullptr;
	HephSubmitInfo m_submitInfo = nullptr;
	HephPresentInfoKHR m_presentInfo = nullptr;
	std::vector<HephAttachmentDescription_T> m_attachments;
	std::vector<HephClearValue_T> m_clearValues;
	std::vector<HephSubpassDescription_T> m_subpassDescriptions;
	std::vector<HephSubpassDependency_T> m_dependencies;

	class HQueue* m_targetQueue = nullptr;
	class HCommandBuffer* m_commandBuffer = nullptr;

	HephSemaphore m_imageReadySem = H_NULL_HANDLE;

	HephFence m_workDoneFence = H_NULL_HANDLE;

	uint32 m_currentFramebufferIndex = H_INVALID;

	std::vector<HephFramebuffer> m_framebuffers;
	HephImageView* m_fbViews = nullptr;
	HephImageView m_depthView = H_NULL_HANDLE;
	uint32 m_numFbViews = H_INVALID;
	HephImage m_depthImage = H_NULL_HANDLE;
	HephDeviceMemory m_depthMemory = H_NULL_HANDLE;

	std::vector<HephCommandBuffer> m_drawCmds;
	std::vector<HAttachmentInfo> m_attachmentMap;
	std::vector<uint32> m_subpassMap;

	uint32 m_currentSubpassIndex;
	bool m_hasPresentationAttachment = false;

	std::vector<HephSemaphore> m_waitSemaphores;
	std::vector<HephSemaphore> m_signalSemaphores;
	std::vector<uint32> m_waitDstStages;

	uint32 m_renderPassName = H_INVALID;
	void ExecuteCommands();
};
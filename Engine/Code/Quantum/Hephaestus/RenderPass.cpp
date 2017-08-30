#include "Quantum/Hephaestus/RenderPass.h"
#include "Quantum/Hephaestus/Manager.h"
#include "Quantum/Hephaestus/LogicalDevice.h"
#include "Quantum/Hephaestus/Swapchain.h"
#include "Quantum/Hephaestus/CommandBuffer.h"
#include "Quantum/Hephaestus/PhysicalDevice.h"
#include "Quantum/Core/String.h"

#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <vulkan.h>


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HRenderPass::HRenderPass(HQueue* targetQueue, const QuString& xmlName)
	: m_targetQueue(targetQueue)
	, m_renderPassName(xmlName)
{
	InitializeFromXML(xmlName);
	
	HLogicalDevice* device = HManager::GetLogicalDevice();

	H_ASSERT(vkCreateRenderPass(*device, m_createInfo, nullptr, &m_renderPass), "Could not create default render pass\n");

	m_beginInfo->renderPass = m_renderPass;
	m_inheritanceInfo->renderPass = m_renderPass;

	CreateFramebuffers();

	GetCommandBuffer();

	InitializeSynchronizationPrimitives();

	InitializeWorkInfos();
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
STATIC HRenderPass* HRenderPass::Create(HQueue* targetQueue, const QuString& xmlName)
{
	HRenderPass* result = new HRenderPass(targetQueue, xmlName);

	HManager::RegisterRenderPass(xmlName, result);

	return result;
}


//-----------------------------------------------------------------------------------------------
HSubpass HRenderPass::RetrieveSubpass(uint32 nameHash)
{
	HSubpass result;
	result.renderPass = m_renderPass;
	ASSERT_OR_DIE(FindSubpassIndex(nameHash, &result.subpassIndex), "Invalid subpass name\n");

	return result;
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::Begin()
{
	m_currentSubpassIndex = 0;
	HManager::SetRenderPass(this);
	HLogicalDevice* device = HManager::GetLogicalDevice();
	HSwapchain* swapchain = HManager::GetSwapchain();

	if (m_hasPresentationAttachment)
	{
		H_ASSERT(vkAcquireNextImageKHR(*device, *swapchain, 0, m_imageReadySem, H_NULL_HANDLE, &m_currentFramebufferIndex), "Could not get next image index\n");
	}
	else
	{
		m_currentFramebufferIndex = 0;
	}

	vkWaitForFences(*device, 1, &m_workDoneFence, VK_TRUE, UINT64_MAX);	//We have to wait here to make sure our command buffer is free to use
	vkResetFences(*device, 1, &m_workDoneFence);	//Now we can reset it so we can wait after the next command buffer submission

	vkResetCommandBuffer(*m_commandBuffer, 0);
	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;
	vkBeginCommandBuffer(*m_commandBuffer, &commandBufferBeginInfo);

	SelectFramebuffer();

	vkCmdBeginRenderPass(*m_commandBuffer, m_beginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);	//Executing secondary buffers for draws

	m_inheritanceInfo->subpass = 0;	//Will always be true at the beginning of the frame
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::Complete()
{
	ExecuteCommands();
	vkCmdEndRenderPass(*m_commandBuffer);
	H_ASSERT(vkEndCommandBuffer(*m_commandBuffer), "Could not finalize render pass command buffer\n");

	H_ASSERT(vkQueueSubmit(*m_targetQueue, 1, m_submitInfo, m_workDoneFence), "Could not submit work to queue\n");

	if (m_hasPresentationAttachment)
	{
		H_ASSERT(vkQueuePresentKHR(*m_targetQueue, m_presentInfo), "Could not present framebuffer to surface\n");
	}
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::NextSubpass()
{
	ExecuteCommands();
	
	vkCmdNextSubpass(*m_commandBuffer, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	m_inheritanceInfo->subpass = ++m_currentSubpassIndex;
}


//-----------------------------------------------------------------------------------------------
HephImageView HRenderPass::GetViewForAttachment(uint32 attachmentName)
{
	HAttachmentInfo* info = FindAttachmentInfo(attachmentName, nullptr);
	ASSERT_OR_DIE(info->views.size() == 1, "Cannot use presentation attachment as shader read texture\n");

	return info->views[0];
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::GetCommandBuffer()
{
	m_commandBuffer = m_targetQueue->RetrieveCommandBuffer(false, true);
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::InitializeSynchronizationPrimitives()
{
	VkSemaphoreCreateInfo semCreateInfo;
	semCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semCreateInfo.pNext = nullptr;
	semCreateInfo.flags = 0;

	HLogicalDevice* device = HManager::GetLogicalDevice();

	//We need two semaphores.  One to signal from image acquisition and wait on queue submission, the other to signal on queue submission and wait on presentation
	if (m_framebuffers.size() > 1)
	{
		//This is only needed if this render pass has multiple framebuffers (i.e. is presentable)
		H_ASSERT(vkCreateSemaphore(*device, &semCreateInfo, nullptr, &m_imageReadySem), "Could not create image-ready semaphore\n");
		GenerateSemaphore();
	}

	VkFenceCreateInfo fenceCreateInfo;
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.pNext = nullptr;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;	//Start signaled so the first wait call returns immediately

	//This fence will allow us to make sure the work from the previous frame is done before we try to reuse the command buffer
	H_ASSERT(vkCreateFence(*device, &fenceCreateInfo, nullptr, &m_workDoneFence), "Could not create work-done fence\n");
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::InitializeWorkInfos()
{
	m_submitInfo = new VkSubmitInfo();
	m_submitInfo->sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	m_submitInfo->pNext = nullptr;
	m_submitInfo->commandBufferCount = 1;
	m_submitInfo->pCommandBuffers = &m_commandBuffer->GetCommandBuffer();	//Can't use my operator here, cause we need a pointer
	m_waitDstStages.push_back(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
	m_submitInfo->pWaitDstStageMask = &m_waitDstStages[0];
	if (m_framebuffers.size() > 1)
	{
		//Only needed if this render pass has a presentation attachment
		m_waitSemaphores.push_back(m_imageReadySem);
		m_submitInfo->waitSemaphoreCount = m_waitSemaphores.size();
		m_submitInfo->pWaitSemaphores = &m_waitSemaphores[0];
		m_submitInfo->signalSemaphoreCount = m_signalSemaphores.size();
		m_submitInfo->pSignalSemaphores = &m_signalSemaphores[0];
	}
	else
	{
		m_submitInfo->waitSemaphoreCount = 0;
		m_submitInfo->pWaitSemaphores = nullptr;
	}

	//We'll only use this one if we have a presentation attachment.  Otherwise, we'll just sit on it.  No harm
	m_presentInfo = new VkPresentInfoKHR();
	m_presentInfo->sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	m_presentInfo->pNext = nullptr;
	m_presentInfo->pImageIndices = &m_currentFramebufferIndex;
	m_presentInfo->pResults = nullptr;
	if (!m_signalSemaphores.empty())
	{
		m_presentInfo->waitSemaphoreCount = 1;
		m_presentInfo->pWaitSemaphores = &m_signalSemaphores[0];
	}
	m_presentInfo->swapchainCount = 1;
	HSwapchain* swapchain = HManager::GetSwapchain();
	m_presentInfo->pSwapchains = &swapchain->GetSwapchain();
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::SelectFramebuffer()
{
	m_beginInfo->framebuffer = m_framebuffers[m_currentFramebufferIndex];
	m_inheritanceInfo->framebuffer = m_framebuffers[m_currentFramebufferIndex];
}


//-----------------------------------------------------------------------------------------------
static VkImageAspectFlags GetAspectMaskFromFormat(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return VK_IMAGE_ASPECT_COLOR_BIT;
	case VK_FORMAT_D32_SFLOAT:
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	case VK_FORMAT_D24_UNORM_S8_UINT:
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	default:
		ERROR_AND_DIE("Unsupported image format\n");
	}
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::CreateFramebuffers()
{

	HSwapchain* swapchain = HManager::GetSwapchain();
	HLogicalDevice* device = HManager::GetLogicalDevice();
	HPhysicalDevice* gpu = HManager::GetPhysicalDevice();

	uint32 attachmentCount = m_attachmentMap.size();
	uint32 numFramebuffers = 0;
	FOR_COUNT(attachmentIndex, attachmentCount)
	{
		HAttachmentInfo& thisAttachment = m_attachmentMap[attachmentIndex];
		
		if (thisAttachment.needsCreate)
		{
			//This is a custom attachment for this render pass, so we'll need to create it
			VkImageCreateInfo imageCreateInfo;
			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCreateInfo.pNext = nullptr;
			imageCreateInfo.flags = 0;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.extent.height = thisAttachment.height;
			imageCreateInfo.extent.width = thisAttachment.width;
			imageCreateInfo.extent.depth = thisAttachment.depth;
			imageCreateInfo.format = thisAttachment.format;
			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageCreateInfo.mipLevels = 1;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageCreateInfo.pQueueFamilyIndices = nullptr;
			imageCreateInfo.queueFamilyIndexCount = 0;
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			uint32 usageFlags = 0;
			uint32 usageFlagCount = thisAttachment.usageFlags.size();
			FOR_COUNT(usageFlagIndex, usageFlagCount)
			{
				usageFlags |= thisAttachment.usageFlags[usageFlagIndex];
			}
			imageCreateInfo.usage = usageFlags;
			
			thisAttachment.images.resize(1);
	
			H_ASSERT(vkCreateImage(*device, &imageCreateInfo, nullptr, &thisAttachment.images[0]), "Could not create image for attachment\n");

			VkMemoryRequirements memReq;
			vkGetImageMemoryRequirements(*device, thisAttachment.images[0], &memReq);

			VkDeviceMemory memory;
			VkMemoryAllocateInfo allocInfo;
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.pNext = nullptr;
			allocInfo.allocationSize = memReq.size;
			allocInfo.memoryTypeIndex = gpu->GetMemoryTypeIndex(memReq.memoryTypeBits, H_MEMORY_TYPE_DEVICE_LOCAL);

			H_ASSERT(vkAllocateMemory(*device, &allocInfo, nullptr, &memory), "Could not allocate memory for attachment\n");
			H_ASSERT(vkBindImageMemory(*device, thisAttachment.images[0], memory, 0), "Could not bind attachment to memory\n");
			numFramebuffers = Max(numFramebuffers, 1);
		}
		else
		{
			//This is a presentation image, so we need to get the images from the swapchain
			uint32 numSwapchainImages;
			H_ASSERT(vkGetSwapchainImagesKHR(*device, *swapchain, &numSwapchainImages, nullptr), "Could not get swapchain image count\n");
			thisAttachment.images.resize(numSwapchainImages);
			H_ASSERT(vkGetSwapchainImagesKHR(*device, *swapchain, &numSwapchainImages, &thisAttachment.images[0]), "Could not get swapchain images\n");
			numFramebuffers = Max(numFramebuffers, numSwapchainImages);
		}

		//Now, we need to create image views for each image in this attachment
		uint32 imageCount = thisAttachment.images.size();
		thisAttachment.views.resize(imageCount);
		FOR_COUNT(imageIndex, imageCount)
		{
			VkImageViewCreateInfo viewCreateInfo;
			viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCreateInfo.pNext = nullptr;
			viewCreateInfo.flags = 0;
			viewCreateInfo.components = {};	//Identity swizzle
			viewCreateInfo.format = thisAttachment.format;
			viewCreateInfo.image = thisAttachment.images[imageIndex];
			viewCreateInfo.subresourceRange.aspectMask = GetAspectMaskFromFormat(viewCreateInfo.format);
			viewCreateInfo.subresourceRange.baseArrayLayer = 0;
			viewCreateInfo.subresourceRange.baseMipLevel = 0;
			viewCreateInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
			viewCreateInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

			H_ASSERT(vkCreateImageView(*device, &viewCreateInfo, nullptr, &thisAttachment.views[imageIndex]), "Could not create view for attachment image\n");
		}
	}
	m_framebuffers.resize(numFramebuffers);
	FOR_COUNT(framebufferIndex, numFramebuffers)
	{
		VkFramebufferCreateInfo framebufferCreateInfo;
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext = nullptr;
		framebufferCreateInfo.flags = 0;
		framebufferCreateInfo.attachmentCount = attachmentCount;
		framebufferCreateInfo.height = m_attachmentMap.data()->height;	//These should have identical dimensions, so the first is fine
		framebufferCreateInfo.width = m_attachmentMap.data()->width;
		framebufferCreateInfo.layers = 1;
		framebufferCreateInfo.renderPass = m_renderPass;

		std::vector<VkImageView> views;
		FOR_COUNT(attachmentIndex, attachmentCount)
		{
			HAttachmentInfo& info = m_attachmentMap[attachmentIndex];
			if (info.views.size() == 1)
			{
				//This is a custom image, so we'll use the same view for each framebuffer
				views.push_back(info.views[0]);
			}
			else
			{
				//This is a swapchain image, so we need to use the view at this framebuffer index
				views.push_back(info.views[framebufferIndex]);
			}
		}

		framebufferCreateInfo.pAttachments = &views[0];

		H_ASSERT(vkCreateFramebuffer(*device, &framebufferCreateInfo, nullptr, &m_framebuffers[framebufferIndex]), "Could not create framebuffer\n");
	}
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::InitializeFromXML(const QuString& xmlName)
{
	m_createInfo = new VkRenderPassCreateInfo();
	m_createInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	m_createInfo->pNext = nullptr;
	m_createInfo->flags = 0;
	m_createInfo->dependencyCount = 0;
	m_createInfo->pDependencies = nullptr;	//Initializing to nothing, because we may not need it

	m_beginInfo = new VkRenderPassBeginInfo();
	m_beginInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	m_beginInfo->pNext = nullptr;
	m_beginInfo->framebuffer = H_NULL_HANDLE;
	m_beginInfo->clearValueCount = 0;
	m_beginInfo->pClearValues = nullptr;	//Initializing to nothing, because we may not need it
	m_beginInfo->renderArea.offset.x = 0;
	m_beginInfo->renderArea.offset.y = 0;
	m_beginInfo->renderArea.extent.width = HManager::GetWindowWidth();
	m_beginInfo->renderArea.extent.height = HManager::GetWindowHeight();

	m_inheritanceInfo = new VkCommandBufferInheritanceInfo();
	m_inheritanceInfo->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	m_inheritanceInfo->pNext = nullptr;
	m_inheritanceInfo->occlusionQueryEnable = VK_FALSE;
	m_inheritanceInfo->queryFlags = 0;
	m_inheritanceInfo->pipelineStatistics = 0;
	m_inheritanceInfo->renderPass = m_renderPass;
	m_inheritanceInfo->subpass = 0;
	m_inheritanceInfo->framebuffer = H_NULL_HANDLE;	//We don't know this yet

	const int32 k_attachmentIndex = 0;
	const int32 k_subpassIndex = 1;
	const int32 k_dependencyIndex = 2;

	QuString filepath = QuString::F("Data/RenderPasses/%s.RenderPass.xml", xmlName.GetRaw());

	XMLNode root = XMLUtils::GetRootNode(filepath);
	ASSERT_OR_DIE(QuString(root.getName()) == "RenderPass", "Expected RenderPass root node\n");

	uint32 childCount = root.nChildNode();
	
	ASSERT_OR_DIE(childCount > k_attachmentIndex, "Too few nodes.  Expected Attachments node\n");
	XMLNode attachmentsNode = root.getChildNode(k_attachmentIndex);
	ASSERT_OR_DIE(QuString(attachmentsNode.getName()) == "Attachments", "Expected Attachments node\n");
	ParseAttachmentsNode(attachmentsNode);

	ASSERT_OR_DIE(childCount > k_subpassIndex, "Too few nodes.  Expected Subpasses node\n");
	XMLNode subpassesNode = root.getChildNode(k_subpassIndex);
	ASSERT_OR_DIE(QuString(subpassesNode.getName()) == "Subpasses", "Expected Subpasses node\n");
	ParseSubpassesNode(subpassesNode);

	if (childCount > k_dependencyIndex)
	{
		ASSERT_OR_DIE(childCount <= k_dependencyIndex + 1, "Too many child nodes of RenderPass\n");
		XMLNode dependenciesNode = root.getChildNode(k_dependencyIndex);
		ASSERT_OR_DIE(QuString(dependenciesNode.getName()) == "Dependencies", "Expected Dependencies node\n");
		ParseDependenciesNode(dependenciesNode);
	}
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::ParseAttachmentsNode(const XMLNode& node)
{
	uint32 attachmentCount = node.nChildNode();

	FOR_COUNT(attachmentIndex, attachmentCount)
	{
		XMLNode attachmentNode = node.getChildNode(attachmentIndex);
		ASSERT_OR_DIE(QuString(attachmentNode.getName()) == "Attachment", "Expected Attachment node\n");
		ParseAttachmentNode(attachmentNode);
	}

	m_createInfo->attachmentCount = m_attachments.size();
	m_createInfo->pAttachments = &m_attachments[0];
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::ParseAttachmentNode(const XMLNode& node)
{
	XML_EXTRACT_ATTRIBUTE(node, name);
	XML_EXTRACT_ATTRIBUTE(node, type);
	XML_EXTRACT_ATTRIBUTE(node, clearColor);
	XML_EXTRACT_ATTRIBUTE(node, clearDepthStencil);
	XML_EXTRACT_ATTRIBUTE(node, format);

	ASSERT_OR_DIE(!name.IsEmpty(), "Must provide attachment name\n");
	ASSERT_OR_DIE(!type.IsEmpty(), "Must provide attachment type\n");
	ASSERT_OR_DIE(!(!clearColor.IsEmpty() && !clearDepthStencil.IsEmpty()), "Cannot provide both clear color and clear depth stencil value\n");

	VkAttachmentDescription attachmentDesc;
	attachmentDesc.flags = 0;
	attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	bool shouldCreateImage = true;
	if (type == "Color")
	{
		attachmentDesc.format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else if (type == "Depth")
	{
		attachmentDesc.format = VK_FORMAT_D32_SFLOAT;
	}
	else if (type == "DepthStencil")
	{
		attachmentDesc.format = VK_FORMAT_D24_UNORM_S8_UINT;
	}
	else if (type == "Present")
	{
		m_hasPresentationAttachment = true;	//Setting this bool to true makes the Complete method perform a Present call
		attachmentDesc.format = VK_FORMAT_B8G8R8A8_UNORM;
		attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		shouldCreateImage = false;
	}
	else if (type == "ResourceDepth")
	{
		attachmentDesc.format = VK_FORMAT_D32_SFLOAT;
		attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	else
	{
		ERROR_AND_DIE("Unrecognized attachment type\n");
	}

	XML_SET_VAR_ONE_OPTION_ERROR_AND_IGNORE(format, attachmentDesc.format,
		"rgba16", VK_FORMAT_R16G16B16A16_SFLOAT,
		"Unrecognized format option\n",
		"");


	HAttachmentInfo attachmentInfo;
	attachmentInfo.format = attachmentDesc.format;
	attachmentInfo.width = m_beginInfo->renderArea.extent.width;
	attachmentInfo.height = m_beginInfo->renderArea.extent.height;
	attachmentInfo.depth = 1;
	attachmentInfo.nameHash = name;
	attachmentInfo.needsCreate = shouldCreateImage;

	//MAYBE WANT TO MOVE THIS ELSEWHERE
	XML_EXTRACT_ATTRIBUTE(node, expectedUsage);
	if (expectedUsage == "sampled")
	{
		attachmentInfo.usageFlags.push_back(VK_IMAGE_USAGE_SAMPLED_BIT);
	}

	m_attachmentMap.push_back(attachmentInfo);

	if (!clearColor.IsEmpty())
	{
		float4 color = clearColor.AsVec4();
		HephClearValue_T clearValue;
		memcpy(clearValue.color.float32, &color, sizeof(float4));
		m_clearValues.push_back(clearValue);
		attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	}
	else if (!clearDepthStencil.IsEmpty())
	{
		if (clearDepthStencil == "true")
		{
			HephClearValue_T clearValue;
			clearValue.depthStencil.depth = 1.f;
			clearValue.depthStencil.stencil = 0;
			m_clearValues.push_back(clearValue);
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		}
		else
		{
			ERROR_AND_DIE("clearDepthStencil can only be true.  If false, omit the attribute\n");
		}
	}
	m_attachments.push_back(attachmentDesc);

	m_beginInfo->clearValueCount = m_clearValues.size();
	m_beginInfo->pClearValues = &m_clearValues[0];
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::ParseSubpassesNode(const XMLNode& node)
{
	uint32 subpassCount = node.nChildNode();

	FOR_COUNT(subpassIndex, subpassCount)
	{
		XMLNode subpassNode = node.getChildNode(subpassIndex);
		ASSERT_OR_DIE(QuString(subpassNode.getName()) == "Subpass", "Expected Subpass node\n");

		ParseSubpassNode(subpassNode);
	}
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::ParseSubpassNode(const XMLNode& node)
{
	ASSERT_OR_DIE(node.nChildNode() == 1, "Can only have one child node of Subpass, which should be Attachments\n");
	XMLNode attachmentsNode = node.getChildNode();
	ASSERT_OR_DIE(QuString(attachmentsNode.getName()) == "Attachments", "Expected Attachments node\n");

	XML_EXTRACT_ATTRIBUTE(node, name);
	ASSERT_OR_DIE(!name.IsEmpty(), "Must supply subpass name\n");

	VkSubpassDescription subpassDesc = {};
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	ParseSubpassAttachmentsNode(attachmentsNode, &subpassDesc);

	m_subpassDescriptions.push_back(subpassDesc);
	m_createInfo->subpassCount = m_subpassDescriptions.size();
	m_createInfo->pSubpasses = &m_subpassDescriptions[0];

	m_subpassMap.push_back(name);
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::ParseSubpassAttachmentsNode(const XMLNode& node, HephSubpassDescription subpassDesc)
{
	std::vector<VkAttachmentReference> inputAttachments;
	std::vector<VkAttachmentReference> colorAttachments;
	std::vector<VkAttachmentReference> depthAttachments;
	std::vector<VkAttachmentReference> preserveAttachments;
	std::vector<VkAttachmentReference> resolveAttachments;

	uint32 attachmentCount = node.nChildNode();

	FOR_COUNT(attachmentIndex, attachmentCount)
	{
		XMLNode attachmentNode = node.getChildNode(attachmentIndex);

		QuString name = attachmentNode.getName();
		if (name == "ColorAttachment")
		{
			ParseSubpassAttachmentNode(attachmentNode, colorAttachments);
		}
		else if (name == "DepthStencilAttachment")
		{
			ParseSubpassAttachmentNode(attachmentNode, depthAttachments);
		}
		else if (name == "InputAttachment" || name == "ReadAttachment")
		{
			ParseSubpassAttachmentNode(attachmentNode, inputAttachments);
		}
		else if (name == "PreserveAttachment")
		{
			ParseSubpassAttachmentNode(attachmentNode, preserveAttachments);
		}
		else if (name == "ResolveAttachment")
		{
			ParseSubpassAttachmentNode(attachmentNode, resolveAttachments);
		}
		else
		{
			ERROR_AND_DIE("Unrecognized attachment type\n");
		}
	}

	if (!inputAttachments.empty())
	{
		subpassDesc->inputAttachmentCount = inputAttachments.size();
		subpassDesc->pInputAttachments = &inputAttachments[0];
	}
	if (!colorAttachments.empty())
	{
		subpassDesc->colorAttachmentCount = colorAttachments.size();
		subpassDesc->pColorAttachments = &colorAttachments[0];

		if (!resolveAttachments.empty())
		{
			ASSERT_OR_DIE(resolveAttachments.size() == subpassDesc->colorAttachmentCount, "Resolve attachment count must equal color attachment count\n");
			subpassDesc->pResolveAttachments = &resolveAttachments[0];
		}
	}
	if (!depthAttachments.empty())
	{
		ASSERT_OR_DIE(depthAttachments.size() == 1, "Only one depth stencil attachment allowed\n");
		subpassDesc->pDepthStencilAttachment = &depthAttachments[0];
	}
	if (!preserveAttachments.empty())
	{
		std::vector<uint32> preserveIndices;
		subpassDesc->preserveAttachmentCount = preserveAttachments.size();
		FOR_COUNT(attIndex, subpassDesc->preserveAttachmentCount)
		{
			preserveIndices.push_back(preserveAttachments[attIndex].attachment);
		}
		subpassDesc->pPreserveAttachments = &preserveIndices[0];
	}
}


//-----------------------------------------------------------------------------------------------
template<typename T>
static void InsertUnique(std::vector<T>& vec, T toInsert)
{
	uint32 count = vec.size();

	FOR_COUNT(index, count)
	{
		if (vec[index] == toInsert)
		{
			return;
		}
	}

	vec.push_back(toInsert);
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::ParseSubpassAttachmentNode(const XMLNode& node, std::vector<HephAttachmentReference_T>& outAttachmentReferences)
{
	XML_EXTRACT_ATTRIBUTE(node, name);
	XML_EXTRACT_ATTRIBUTE(node, layout);

	uint32 attachmentIndex;
	HAttachmentInfo* info = FindAttachmentInfo(name, &attachmentIndex);
	ASSERT_OR_DIE(info, "Attachment name invalid\n");

	VkAttachmentReference reference;

	if (layout == "")
	{
		reference.layout = VK_IMAGE_LAYOUT_UNDEFINED;
	}
	else if (layout == "color")
	{
		reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		InsertUnique(info->usageFlags, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	}
	else if (layout == "depthStencil")
	{
		reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		InsertUnique(info->usageFlags, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
	else if (layout == "inputRead")
	{
		reference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		InsertUnique(info->usageFlags, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
	}
	else if (layout == "depthRead")
	{
		reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		InsertUnique(info->usageFlags, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
	else if (layout == "shaderRead")
	{
		reference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		InsertUnique(info->usageFlags, VK_IMAGE_USAGE_SAMPLED_BIT);
		InsertUnique(info->usageFlags, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
	}
	else
	{
		ERROR_AND_DIE("Unrecognized layout attribute\n");
	}

	reference.attachment = attachmentIndex;

	outAttachmentReferences.push_back(reference);
}


//-----------------------------------------------------------------------------------------------
HAttachmentInfo* HRenderPass::FindAttachmentInfo(uint32 name, uint32* pOutAttachmentIndex)
{
	uint32 attachmentCount = m_attachmentMap.size();

	FOR_COUNT(attachmentIndex, attachmentCount)
	{
		HAttachmentInfo* currInfo = &m_attachmentMap[attachmentIndex];
		if (currInfo->nameHash == name)
		{
			if (pOutAttachmentIndex)
			{
				*pOutAttachmentIndex = attachmentIndex;
			}
			return currInfo;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::ParseDependenciesNode(const XMLNode& node)
{
	uint32 dependencyCount = node.nChildNode();

	FOR_COUNT(dependencyIndex, dependencyCount)
	{
		XMLNode dependencyNode = node.getChildNode(dependencyIndex);

		ASSERT_OR_DIE(QuString(dependencyNode.getName()) == "Dependency", "Expected Dependency node\n");
		ParseDependencyNode(dependencyNode);
	}

	m_createInfo->dependencyCount = m_dependencies.size();
	m_createInfo->pDependencies = &m_dependencies[0];
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::ParseDependencyNode(const XMLNode& node)
{
	VkSubpassDependency dependency;

	XML_EXTRACT_ATTRIBUTE(node, localized);
	XML_SET_VAR_TWO_OPTIONS_WITH_ERROR(localized, dependency.dependencyFlags,
		"", 0,
		"true", VK_DEPENDENCY_BY_REGION_BIT,
		"localized attribute can only be null or true\n");

	uint32 subpassIndex;
	uint32 stageMask;
	uint32 accessMask;
	XMLNode srcNode = node.getChildNode();
	ASSERT_OR_DIE(QuString(srcNode.getName()) == "Source", "Expected Source node\n");
	GetDependencyValuesFromNode(srcNode, &subpassIndex, &stageMask, &accessMask);
	dependency.srcSubpass = subpassIndex;
	dependency.srcStageMask = stageMask;
	dependency.srcAccessMask = accessMask;
	XMLNode dstNode = node.getChildNode(1);
	ASSERT_OR_DIE(QuString(dstNode.getName()) == "Target", "Expected Target node\n");
	GetDependencyValuesFromNode(dstNode, &subpassIndex, &stageMask, &accessMask);
	dependency.dstSubpass = subpassIndex;
	dependency.dstStageMask = stageMask;
	dependency.dstAccessMask = accessMask;

	m_dependencies.push_back(dependency);
}


void HRenderPass::GetDependencyValuesFromNode(const XMLNode node, uint32* pOutSubpassIndex, uint32* pOutStageMask, uint32* pOutAccessMask)
{
	XML_EXTRACT_ATTRIBUTE(node, subpass);
	uint32 subpassIndex;
	ASSERT_OR_DIE(!subpass.IsEmpty(), "Must provide subpass name\n");
	ASSERT_OR_DIE(FindSubpassIndex(subpass, &subpassIndex), "Subpass name does not exist\n");
	if (pOutSubpassIndex)
	{
		*pOutSubpassIndex = subpassIndex;
	}

	XML_EXTRACT_ATTRIBUTE(node, usage);
	ASSERT_OR_DIE(!usage.IsEmpty(), "Must provide usage\n");
	VkPipelineStageFlags stageMask;
	VkAccessFlags accessMask;
	if (usage == "depthWrite")
	{
		stageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;	//Only guaranteed bottleneck for depth write
		accessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	else if (usage == "depthRead")
	{
		stageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		accessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	}
	else if (usage == "colorAttachmentWrite")
	{
		stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		accessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	else if (usage == "fragmentRead")
	{
		stageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		accessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	else
	{
		ERROR_AND_DIE("Unrecognized usage attribute\n");
	}

	if (pOutStageMask)
	{
		*pOutStageMask = stageMask;
	}
	if (pOutAccessMask)
	{
		*pOutAccessMask = accessMask;
	}
}


//-----------------------------------------------------------------------------------------------
bool HRenderPass::FindSubpassIndex(uint32 subpassName, uint32* pOutIndex)
{
	uint32 subpassCount = m_subpassMap.size();

	FOR_COUNT(subpassIndex, subpassCount)
	{
		if (m_subpassMap[subpassIndex] == subpassName)
		{
			if (pOutIndex)
			{
				*pOutIndex = subpassIndex;
			}
			
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
HephSemaphore HRenderPass::GenerateSemaphore()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo;
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	semaphoreCreateInfo.flags = 0;

	m_signalSemaphores.push_back(0);
	
	HLogicalDevice* device = HManager::GetLogicalDevice();

	H_ASSERT(vkCreateSemaphore(*device, &semaphoreCreateInfo, nullptr, &m_signalSemaphores[m_signalSemaphores.size() - 1]), "Could not create signal semaphore\n");

	if (m_submitInfo)
	{
		m_submitInfo->signalSemaphoreCount = m_signalSemaphores.size();
		m_submitInfo->pSignalSemaphores = &m_signalSemaphores[0];
	}

	return m_signalSemaphores.back();
}


//-----------------------------------------------------------------------------------------------
void HRenderPass::ExecuteCommands()
{
	if (!m_drawCmds.empty())
	{
		vkCmdExecuteCommands(*m_commandBuffer, m_drawCmds.size(), &m_drawCmds[0]);
		m_drawCmds.clear();
	}
}


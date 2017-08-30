#include "Quantum/Hephaestus/CommandBuffer.h"
#include "Quantum/Hephaestus/LogicalDevice.h"
#include "Quantum/Hephaestus/CommandPool.h"
#include "Quantum/Hephaestus/Queue.h"
#include "Quantum/Hephaestus/Manager.h"

#include <vulkan.h>


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HCommandBuffer::HCommandBuffer(HLogicalDevice* device, HCommandPool* pool, bool isPrimary)
	: m_isPrimary(isPrimary)
{
	VkCommandBufferAllocateInfo bufferAllocateInfo;
	bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	bufferAllocateInfo.pNext = nullptr;
	bufferAllocateInfo.commandBufferCount = 1;
	bufferAllocateInfo.commandPool = pool->m_pool;
	bufferAllocateInfo.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;

	H_ASSERT(vkAllocateCommandBuffers(*device, &bufferAllocateInfo, &m_buffer), "Could not allocate command buffer\n");

	VkFenceCreateInfo fenceCreateInfo =
	{
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		nullptr,
		0
	};
	H_ASSERT(vkCreateFence(*device, &fenceCreateInfo, nullptr, &m_fence), "Could not create fence\n");
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::Begin(HephCommandBufferInheritanceInfo inheritanceInfo /* = nullptr */)
{
	VkCommandBufferBeginInfo bufferBeginInfo;
	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	bufferBeginInfo.pNext = nullptr;
	bufferBeginInfo.flags = m_isPrimary ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;	//If secondary, may need to use RENDER_PASS_CONTINUE_BIT
	bufferBeginInfo.pInheritanceInfo = inheritanceInfo;

	H_ASSERT(vkBeginCommandBuffer(m_buffer, &bufferBeginInfo), "Could not begin command buffer\n");
}


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::End()
{
	H_ASSERT(vkEndCommandBuffer(m_buffer), "Could not end command buffer\n");
}


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::Submit()
{
	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_buffer;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	H_ASSERT(vkQueueSubmit(*m_queue, 1, &submitInfo, m_fence), "Could not submit work to queue\n");
}


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::Wait()
{
	HLogicalDevice* device = HManager::GetLogicalDevice();
	H_ASSERT(vkWaitForFences(*device, 1, &m_fence, VK_TRUE, UINT64_MAX), "Could not wait for fence\n");
}


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::Reset(bool releaseResources /* = false */)
{
	HLogicalDevice* device = HManager::GetLogicalDevice();
	H_ASSERT(vkResetFences(*device, 1, &m_fence), "Could not reset fence\n");
	H_ASSERT(vkResetCommandBuffer(m_buffer, releaseResources ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0), "Could not reset command buffer\n");
}


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::BindVertexBuffer(HephBuffer vertexBuffer)
{
	uint64 zero = 0;
	vkCmdBindVertexBuffers(m_buffer, 0, 1, &vertexBuffer, &zero);	//Potential to bind sparse data (across buffers) and let the assembler interleave them
}


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::BindIndexBuffer(HephBuffer indexBuffer, EIndexType indexType)
{
	vkCmdBindIndexBuffer(m_buffer, indexBuffer, 0, indexType == H_INDEX_TYPE_UINT16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
}


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::BindGraphicsPipeline(HephPipeline pipeline)
{
	vkCmdBindPipeline(m_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::BindComputePipeline(HephPipeline pipeline)
{
	vkCmdBindPipeline(m_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
}


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::BindGraphicsDescriptorSets(const HephDescriptorSet* pDescriptorSets, HephPipelineLayout layout, uint32 numDescriptorSets)
{
	vkCmdBindDescriptorSets(m_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, numDescriptorSets, pDescriptorSets, 0, nullptr);
}


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::BindComputeDescriptorSets(const HephDescriptorSet* pDescriptorSets, HephPipelineLayout layout, uint32 numDescriptorSets)
{
	vkCmdBindDescriptorSets(m_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, layout, 0, numDescriptorSets, pDescriptorSets, 0, nullptr);
}


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::Draw(uint32 numVertices)
{
	vkCmdDraw(m_buffer, numVertices, 1, 0, 0);
}


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::DrawIndexed(uint32 numIndices)
{
	vkCmdDrawIndexed(m_buffer, numIndices, 1, 0, 0, 0);
}


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::Dispatch(uint32 numWorkGroupsX, uint32 numWorkGroupsY, uint32 numWorkGroupsZ)
{
	vkCmdDispatch(m_buffer, numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
}


//-----------------------------------------------------------------------------------------------
uint32 GetAccessFromStage(EPipelineStage stage)
{
	switch (stage)
	{
	case H_PIPELINE_STAGE_TRANSFER_WRITE:
		return VK_ACCESS_TRANSFER_WRITE_BIT;
	case H_PIPELINE_STAGE_FRAGMENT_SHADER_READ:
		return VK_ACCESS_SHADER_READ_BIT;
	case H_PIPELINE_STAGE_HOST:
		return VK_ACCESS_HOST_WRITE_BIT;
	}
}


//-----------------------------------------------------------------------------------------------
VkImageLayout TranslateLayout(EImageLayout layout)
{
	switch (layout)
	{
	case H_IMAGE_LAYOUT_UNDEFINED:
		return VK_IMAGE_LAYOUT_UNDEFINED;
	case H_IMAGE_LAYOUT_SHADER_READ_OPTIMAL:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case H_IMAGE_LAYOUT_TRANSFER_WRITE:
		return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}
}


//-----------------------------------------------------------------------------------------------
uint32 TranslateAspect(EImageAspect aspect)
{
	switch (aspect)
	{
	case H_IMAGE_ASPECT_COLOR:
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}


//-----------------------------------------------------------------------------------------------
uint32 TranslateStage(EPipelineStage stage)
{
	switch (stage)
	{
	case H_PIPELINE_STAGE_TRANSFER_WRITE:
		return VK_PIPELINE_STAGE_TRANSFER_BIT;
	case H_PIPELINE_STAGE_FRAGMENT_SHADER_READ:
		return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	case H_PIPELINE_STAGE_HOST:
		return VK_PIPELINE_STAGE_HOST_BIT;
	}
}


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::InsertImagePipelineBarrier(HephImage image, EImageAspect imageAspect, EImageLayout fromLayout, EImageLayout toLayout, EPipelineStage fromStage, EPipelineStage toStage)
{
	VkImageMemoryBarrier imageBarrier;
	imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrier.pNext = nullptr;
	imageBarrier.srcAccessMask = GetAccessFromStage(fromStage);
	imageBarrier.dstAccessMask = GetAccessFromStage(toStage);
	imageBarrier.image = image;
	imageBarrier.oldLayout = TranslateLayout(fromLayout);
	imageBarrier.newLayout = TranslateLayout(toLayout);
	imageBarrier.subresourceRange.aspectMask = TranslateAspect(imageAspect);
	imageBarrier.subresourceRange.baseArrayLayer = 0;
	imageBarrier.subresourceRange.baseMipLevel = 0;
	imageBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	imageBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	vkCmdPipelineBarrier(m_buffer, TranslateStage(fromStage), TranslateStage(toStage), 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
}


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::CopyBufferToImage(HephBuffer buffer, HephImage image, uint32 width, uint32 height)
{
	VkBufferImageCopy copyInfo;
	copyInfo.bufferOffset = 0;
	copyInfo.bufferImageHeight = 0;
	copyInfo.bufferRowLength = 0;
	copyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	//May want to populate these fields from meta data
	copyInfo.imageSubresource.baseArrayLayer = 0;
	copyInfo.imageSubresource.mipLevel = 0;
	copyInfo.imageSubresource.layerCount = 1;

	copyInfo.imageOffset =
	{
		0, 0, 0
	};

	copyInfo.imageExtent.width = width;
	copyInfo.imageExtent.height = height;
	copyInfo.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(m_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);
}


//-----------------------------------------------------------------------------------------------
void HCommandBuffer::CopyBufferToImageCube(HephBuffer buffer, HephImage image, uint32 width, uint32 height)
{
	VkBufferImageCopy copyInfos[6];
	//-X
	copyInfos[0].bufferOffset = (width * (height / 3) + width / 2) * 4;
	copyInfos[0].bufferImageHeight = height;
	copyInfos[0].bufferRowLength = width;
	copyInfos[0].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyInfos[0].imageSubresource.baseArrayLayer = 0;
	copyInfos[0].imageSubresource.layerCount = 1;
	copyInfos[0].imageSubresource.mipLevel = 0;

	copyInfos[0].imageOffset =
	{
		0, 0, 0
	};

	copyInfos[0].imageExtent.width = width / 4;
	copyInfos[0].imageExtent.height = height / 3;
	copyInfos[0].imageExtent.depth = 1;

	//+X
	copyInfos[1] = copyInfos[0];
	copyInfos[1].bufferOffset -= (width / 2) * 4;
	copyInfos[1].imageSubresource.baseArrayLayer = 1;

	//-Y
	copyInfos[2] = copyInfos[1];
	copyInfos[2].bufferOffset = width / 4 * 4;
	copyInfos[2].imageSubresource.baseArrayLayer = 2;

	//+Y
	copyInfos[3] = copyInfos[1];
	copyInfos[3].bufferOffset = (width * (height / 3) * 2 + (width / 4)) * 4;
	copyInfos[3].imageSubresource.baseArrayLayer = 3;

	//-Z
	copyInfos[4] = copyInfos[1];
	copyInfos[4].bufferOffset += (width / 4) * 4;
	copyInfos[4].imageSubresource.baseArrayLayer = 4;

	//+Z
	copyInfos[5] = copyInfos[0];
	copyInfos[5].bufferOffset += (width / 4) * 4;
	copyInfos[5].imageSubresource.baseArrayLayer = 5;

	vkCmdCopyBufferToImage(m_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6, copyInfos);
}


//-----------------------------------------------------------------------------------------------
void HCommandBufferImmediate::Submit()
{
	HCommandBuffer::Submit();
	Wait();
	Reset();
}


//-----------------------------------------------------------------------------------------------
void HCommandBufferImmediate::InsertImagePipelineBarrier(HephImage image, EImageAspect imageAspect, EImageLayout fromLayout, EImageLayout toLayout, EPipelineStage fromStage, EPipelineStage toStage)
{
	Begin();
	HCommandBuffer::InsertImagePipelineBarrier(image, imageAspect, fromLayout, toLayout, fromStage, toStage);
	End();
	Submit();
}


//-----------------------------------------------------------------------------------------------
void HCommandBufferImmediate::CopyBufferToImage(HephBuffer buffer, HephImage image, uint32 width, uint32 height)
{
	Begin();
	HCommandBuffer::CopyBufferToImage(buffer, image, width, height);
	End();
	Submit();
}


//-----------------------------------------------------------------------------------------------
void HCommandBufferImmediate::CopyBufferToImageCube(HephBuffer buffer, HephImage image, uint32 width, uint32 height)
{
	Begin();
	HCommandBuffer::CopyBufferToImageCube(buffer, image, width, height);
	End();
	Submit();
}
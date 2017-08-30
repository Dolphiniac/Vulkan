#pragma once

#include "Quantum/Hephaestus/Declarations.h"


//-----------------------------------------------------------------------------------------------
class HCommandBuffer
{
	friend class HCommandPool;

public:
	void Begin(HephCommandBufferInheritanceInfo inheritanceInfo = nullptr);
	void End();
	void Reset(bool releaseResources = false);
	virtual void Submit();
	void Wait();

	//-----------------------------------------------------------------------------------------------
	//COMMAND WRAPPERS
	//NOTE: May want to inline these
	//-----------------------------------------------------------------------------------------------
	void BindVertexBuffer(HephBuffer vertexBuffer);
	void BindIndexBuffer(HephBuffer indexBuffer, EIndexType indexType);
	void BindGraphicsPipeline(HephPipeline pipeline);
	void BindComputePipeline(HephPipeline pipeline);
	void BindGraphicsDescriptorSets(const HephDescriptorSet* pDescriptorSets, HephPipelineLayout layout, uint32 numDescriptorSets);
	void BindComputeDescriptorSets(const HephDescriptorSet* pDescriptorSets, HephPipelineLayout layout, uint32 numDescriptorSets);
	void Draw(uint32 numVertices);
	void DrawIndexed(uint32 numIndices);
	void Dispatch(uint32 numWorkGroupsX, uint32 numWorkGroupsY, uint32 numWorkGroupsZ);
	virtual void InsertImagePipelineBarrier(HephImage image, EImageAspect imageAspect, EImageLayout fromLayout, EImageLayout toLayout, EPipelineStage fromStage, EPipelineStage toStage);
	virtual void CopyBufferToImage(HephBuffer buffer, HephImage image, uint32 width, uint32 height);
	virtual void CopyBufferToImageCube(HephBuffer buffer, HephImage image, uint32 width, uint32 height);
	//-----------------------------------------------------------------------------------------------
	//END COMMAND WRAPPERS
	//-----------------------------------------------------------------------------------------------
	operator HephCommandBuffer() const { return m_buffer; }
	const HephCommandBuffer& GetCommandBuffer() const { return m_buffer; }

protected:
	class HQueue* m_queue = nullptr;
	
protected:
	HCommandBuffer(class HLogicalDevice* device, class HCommandPool* pool, bool isPrimary);

private:
	bool m_isPrimary = true;

protected:
	HephFence m_fence = H_NULL_HANDLE;
	HephCommandBuffer m_buffer = H_NULL_HANDLE;
};


//-----------------------------------------------------------------------------------------------
class HCommandBufferImmediate : public HCommandBuffer
{
	friend class HCommandPool;
	friend class HQueue;

public:
	virtual void Submit() override;

	//-----------------------------------------------------------------------------------------------
	//COMMAND WRAPPERS
	//NOTE: May want to inline these
	//-----------------------------------------------------------------------------------------------
	virtual void InsertImagePipelineBarrier(HephImage image, EImageAspect imageAspect, EImageLayout fromLayout, EImageLayout toLayout, EPipelineStage fromStage, EPipelineStage toStage) override;
	virtual void CopyBufferToImage(HephBuffer buffer, HephImage image, uint32 width, uint32 height) override;
	virtual void CopyBufferToImageCube(HephBuffer buffer, HephImage image, uint32 width, uint32 height) override;
	//-----------------------------------------------------------------------------------------------
	//END COMMAND WRAPPERS
	//-----------------------------------------------------------------------------------------------

private:
	HCommandBufferImmediate(class HLogicalDevice* device, class HCommandPool* pool) : HCommandBuffer(device, pool, true) {}
};
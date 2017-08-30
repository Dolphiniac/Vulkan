#pragma once

#include "Quantum/Hephaestus/Declarations.h"

#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
class HMaterialState
{
	friend class HMaterial;

public:
	HMaterialState(class HPipeline* pipeline);
	void Bind(class HCommandBuffer* commandBuffer);
	static HephDescriptorPool s_pool;

private:
	void AllocateDescriptorSets();
	static void InitializeDescriptorPool();
	uint32 FindBindPointFor(uint32 nameHash) const;
	void BindUniformBufferData(const class QuString& name, void* data, uint32 dataSize);
	void BindTexelBufferData(const class QuString& name, void* data, uint32 dataSize);

private:
	std::vector<HephDescriptorSet> m_descriptorSets;
	HMaterialState* m_parent = nullptr;
	class HPipeline* m_pipeline = nullptr;

	std::map<uint32, HephDeviceMemory> m_inUseMemoryBlocks;	//NEEDS REFACTORING!  THIS IS UNIQUE TO A SUBPASS STATE
};
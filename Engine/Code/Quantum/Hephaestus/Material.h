#pragma once

#include "Quantum/Hephaestus/Declarations.h"
#include "Quantum/Core/SimpleMap.h"

#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
class HMaterial
{
	friend class HMeshRenderer;

public:
	HMaterial(class HPipeline* pipeline, uint32 subpassIndex = 0);
	HMaterial(const class QuString& materialHierarchy);

public:
	void BindState(class HCommandBuffer* commandBuffer, uint32 renderPassHandle, uint32 subpassIndex);
	void BindTexture(class HTexture* texture, uint32 bindingIndex, uint32 subpassIndex);
	void BindTexture(class HTexture* texture, const class QuString& name, uint32 subpassIndex);

	//renderPassIndexString is in the form RenderPassName.SubpassName.  If the data needs to be set for multiple pipelines, use the form without this string
	void BindUniformBufferData(const QuString& name, void* data, uint32 dataSize, const QuString& renderPassIndexString);
	void BindUniformBufferData(const QuString& name, void* data, uint32 dataSize);
	void BindTexelBufferData(const QuString& name, void* data, uint32 dataSize, const QuString& renderPassIndexString);
	void BindTexelBufferData(const QuString& name, void* data, uint32 dataSize);
	static HMaterial FromAssociation(const QuString& assocName, const QuString& mappingSrcName);

private:
	void CreateMaterialState(class HPipeline* pipeline, uint32 subpassIndex);
	void InitializeFromXML(const QuString& materialHierarchy);

	void ParseRenderPassNode(const struct XMLNode& node, const QuString& instanceName);

	void ParseBaseNode(const struct XMLNode& node, const QuString& renderPassName);
	void ParseInstanceNode(const XMLNode& node, const QuString& expectedName, const QuString& renderPassName);
	void ParseSubpassNode(const XMLNode& node, bool isBase, const QuString& renderPassName);

	HPipeline* ParsePipelineNode(HPipeline* pipeline, bool isBase, XMLNode settingNode, const QuString& subpassName, const QuString& renderPassName);

	void ParseTextureBindingNode(const XMLNode& node, uint32 subpassIndex);

	void ParseInputAttachmentBindingNode(const XMLNode& node, uint32 subpassIndex, const QuString& renderPassName);
	void ParseReadAttachmentBindingNode(const XMLNode& node, uint32 subpassIndex, const QuString& renderPassName);
	void BindInputAttachment(HephImageView view, uint32 bindingIndex, uint32 subpassIndex);
	void BindInputAttachment(uint32 name, HephImageView view, uint32 subpassIndex);
	void BindReadAttachment(HephImageView view, uint32 bindingIndex, uint32 subpassIndex);
	void BindReadAttachment(uint32 name, HephImageView view, uint32 subpassIndex);
protected:
	QuSimpleMap<std::vector<class HMaterialState*>> m_states;
	std::vector<class HMaterialState*> m_tempSubpassStates;
};
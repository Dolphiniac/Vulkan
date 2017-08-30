#include "Quantum/Hephaestus/Material.h"
#include "Quantum/Hephaestus/MaterialState.h"
#include "Quantum/Hephaestus/Manager.h"
#include "Quantum/Hephaestus/Queue.h"
#include "Quantum/Hephaestus/CommandBuffer.h"
#include "Quantum/Hephaestus/Texture.h"
#include "Quantum/Hephaestus/LogicalDevice.h"
#include "Quantum/Hephaestus/PhysicalDevice.h"
#include "Quantum/Core/String.h"
#include "Quantum/Hephaestus/Pipeline.h"
#include "Quantum/Hephaestus/PipelineGenerator.h"
#include "Engine/Core/XMLUtils.hpp"
#include "Quantum/Hephaestus/RenderPass.h"

#include <vulkan.h>


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HMaterial::HMaterial(HPipeline* pipeline, uint32 subpassIndex /* = 0 */)
{
	CreateMaterialState(pipeline, subpassIndex);
}


//-----------------------------------------------------------------------------------------------
HMaterial::HMaterial(const QuString& materialHierarchy)
{
	InitializeFromXML(materialHierarchy);
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
void HMaterial::CreateMaterialState(HPipeline* pipeline, uint32 subpassIndex)
{
	if (m_tempSubpassStates.size() < subpassIndex + 1)
	{
		//We need to resize the vector, but only expand it.  Never contract
		m_tempSubpassStates.resize(subpassIndex + 1, nullptr);
	}

	m_tempSubpassStates[subpassIndex] = new HMaterialState(pipeline);
}


//-----------------------------------------------------------------------------------------------
void HMaterial::InitializeFromXML(const QuString& materialHierarchy)
{
	std::vector<QuString> hierarchyStrings = materialHierarchy.SplitOnDelimiter('.');
	ASSERT_OR_DIE(hierarchyStrings.size() <= 2, "Cannot parse hierarchy greater than one level deep\n");

	QuString instanceName = "";
	if (hierarchyStrings.size() == 2)
	{
		instanceName = hierarchyStrings[1];
	}
	QuString materialName = hierarchyStrings[0];

	QuString materialFile = QuString::F("Data/Materials/%s.Material.xml", materialName.GetRaw());

	XMLNode root = XMLUtils::GetRootNode(materialFile);

	uint32 renderPassCount = root.nChildNode();
	FOR_COUNT(renderPassIndex, renderPassCount)
	{
		XMLNode renderPassNode = root.getChildNode(renderPassIndex);

		ASSERT_OR_DIE(QuString(renderPassNode.getName()) == "RenderPass", "Expected RenderPass node\n");
		ParseRenderPassNode(renderPassNode, instanceName);
	}
}


//-----------------------------------------------------------------------------------------------
void HMaterial::ParseRenderPassNode(const XMLNode& node, const QuString& instanceName)
{
	XML_EXTRACT_ATTRIBUTE(node, name);

	uint32 materialCount = node.nChildNode();
	bool foundBase = false;
	FOR_COUNT(materialIndex, materialCount)
	{
		XMLNode materialNode = node.getChildNode(materialIndex);

		QuString materialName = materialNode.getName();

		if (materialName == "Base")
		{
			ASSERT_OR_DIE(!foundBase, "Cannot initialize two base materials\n");
			ParseBaseNode(materialNode, name);
			if (instanceName == "")
			{
				//No instancing required, so we can stop here
				break;
			}
			foundBase = true;
		}
		else if (materialName == "Instance")
		{
			ASSERT_OR_DIE(foundBase, "Cannot create instance without base material\n");
			ParseInstanceNode(materialNode, instanceName, name);
		}
		else
		{
			ERROR_AND_DIE("Unrecognized sub-node for material\n");
		}
	}

	//Now that we've filled the subpass states vector, we can store it in states by the render pass name
	m_states.Insert(name, m_tempSubpassStates);

	m_tempSubpassStates.clear();
}


//-----------------------------------------------------------------------------------------------
void HMaterial::ParseBaseNode(const XMLNode& node, const QuString& renderPassName)
{
	uint32 subpassCount = node.nChildNode();
	FOR_COUNT(subpassIndex, subpassCount)
	{
		XMLNode subpassNode = node.getChildNode(subpassIndex);

		ASSERT_OR_DIE(QuString(subpassNode.getName()) == "Subpass", "Subpass node expected\n");
		ParseSubpassNode(subpassNode, true, renderPassName);
	}
}


//-----------------------------------------------------------------------------------------------
void HMaterial::ParseInstanceNode(const XMLNode& node, const QuString& expectedName, const QuString& renderPassName)
{
	XML_EXTRACT_ATTRIBUTE(node, name);
	if (name != expectedName)
	{
		return;
	}

	uint32 subpassCount = node.nChildNode();
	FOR_COUNT(subpassIndex, subpassCount)
	{
		XMLNode subpassNode = node.getChildNode(subpassIndex);

		ASSERT_OR_DIE(QuString(subpassNode.getName()) == "Subpass", "Subpass node expected\n");
		ParseSubpassNode(subpassNode, false, renderPassName);
	}
}


//-----------------------------------------------------------------------------------------------
void HMaterial::ParseSubpassNode(const XMLNode& node, bool isBase, const QuString& renderPassName)
{
	uint32 settingCount = node.nChildNode();
	HPipeline* pipeline = nullptr;

	XML_EXTRACT_ATTRIBUTE(node, name)

	FOR_COUNT(settingIndex, settingCount)
	{
		XMLNode settingNode = node.getChildNode(settingIndex);
		
		QuString settingName = settingNode.getName();

		if (settingName == "Pipeline")
		{
			pipeline = ParsePipelineNode(pipeline, isBase, settingNode, name, renderPassName);

		}
		else if (settingName == "TextureBinding")
		{
			HRenderPass* pass = HManager::GetCurrentRenderPass();
			HSubpass subpass = pass->RetrieveSubpass(name);
			ASSERT_OR_DIE(m_tempSubpassStates.size() > subpass.subpassIndex || !isBase, "Cannot bind texture if pipeline is not bound yet\n");
			ParseTextureBindingNode(settingNode, subpass.subpassIndex);
		}
		else if (settingName == "InputAttachmentBinding")
		{
			HRenderPass* pass = HManager::GetCurrentRenderPass();
			HSubpass subpass = pass->RetrieveSubpass(name);
			ASSERT_OR_DIE(m_tempSubpassStates.size() > subpass.subpassIndex || !isBase, "Cannot bind texture if pipeline is not bound yet\n");
			ParseInputAttachmentBindingNode(settingNode, subpass.subpassIndex, renderPassName);
		}
		else if (settingName == "ReadAttachmentBinding")
		{
			HRenderPass* pass = HManager::GetCurrentRenderPass();
			HSubpass subpass = pass->RetrieveSubpass(name);
			ASSERT_OR_DIE(m_tempSubpassStates.size() > subpass.subpassIndex || !isBase, "Cannot bind texture if pipeline is not bound yet\n");
			ParseReadAttachmentBindingNode(settingNode, subpass.subpassIndex, renderPassName);
		}
		else
		{
			ERROR_AND_DIE("Unrecognized sub-node of Subpass\n");
		}
	}
}


//-----------------------------------------------------------------------------------------------
HPipeline* HMaterial::ParsePipelineNode(HPipeline* pipeline, bool isBase, XMLNode settingNode, const QuString& subpassName, const QuString& renderPassName)
{
	ASSERT_OR_DIE(!pipeline, "Cannot set multiple pipelines for single material subpass\n");
	ASSERT_OR_DIE(isBase, "Cannot set pipeline for material instance\n");
	XML_EXTRACT_ATTRIBUTE(settingNode, name);
	HPipelineGenerator* generator = HManager::GetPipelineGenerator();
	pipeline = generator->CreateOrGetPipeline(name, renderPassName, subpassName);

	CreateMaterialState(pipeline, pipeline->GetSubpassIndex());
	return pipeline;
}


//-----------------------------------------------------------------------------------------------
void HMaterial::ParseTextureBindingNode(const XMLNode& node, uint32 subpassIndex)
{
	XML_EXTRACT_ATTRIBUTE(node, name);
	XML_EXTRACT_ATTRIBUTE(node, src);
	XML_EXTRACT_ATTRIBUTE(node, textureType);

	ETextureType type;

	XML_SET_VAR_THREE_OPTIONS_WITH_ERROR(textureType, type,
		"", H_TEXTURE_TYPE_2D,
		"2D", H_TEXTURE_TYPE_2D,
		"cube", H_TEXTURE_TYPE_CUBE_MAP,
		"Unrecognized texture type\n");

	QuString filepath = QuString::F("Data/Textures/%s", src.GetRaw());

	HTexture* tex = HTexture::CreateOrGetTexture(filepath, type);
	BindTexture(tex, name, subpassIndex);
}


//-----------------------------------------------------------------------------------------------
static HRenderPass* GetRenderPassForSrc(const QuString& src, uint32 renderPassHash, uint32* pOutAttachmentName)
{
	std::vector<QuString> tokens = src.SplitOnDelimiter('.');

	switch (tokens.size())
	{
	case 1:
		if (pOutAttachmentName)
		{
			*pOutAttachmentName = src;
		}
		return HManager::GetRenderPassByName(renderPassHash);
	case 2:
		if (pOutAttachmentName)
		{
			*pOutAttachmentName = tokens[1];
		}
		return HManager::GetRenderPassByName(tokens[0]);
	default:
		ERROR_AND_DIE("Invalid attachment src\n");
	}
}


//-----------------------------------------------------------------------------------------------
void HMaterial::ParseInputAttachmentBindingNode(const XMLNode& node, uint32 subpassIndex, const QuString& renderPassName)
{
	XML_EXTRACT_ATTRIBUTE(node, name);
	XML_EXTRACT_ATTRIBUTE(node, src);

	uint32 attachmentName;
	HRenderPass* renderPass = GetRenderPassForSrc(src, renderPassName, &attachmentName);

	BindInputAttachment(name, renderPass->GetViewForAttachment(attachmentName), subpassIndex);
}


//-----------------------------------------------------------------------------------------------
void HMaterial::ParseReadAttachmentBindingNode(const XMLNode& node, uint32 subpassIndex, const QuString& renderPassName)
{
	XML_EXTRACT_ATTRIBUTE(node, name);
	XML_EXTRACT_ATTRIBUTE(node, src);

	uint32 attachmentName;
	HRenderPass* renderPass = GetRenderPassForSrc(src, renderPassName, &attachmentName);

	BindReadAttachment(name, renderPass->GetViewForAttachment(attachmentName), subpassIndex);
}


//-----------------------------------------------------------------------------------------------
void HMaterial::BindInputAttachment(HephImageView view, uint32 bindingIndex, uint32 subpassIndex)
{
	HephWriteDescriptorSet_T writeDescriptor;
	writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptor.pNext = nullptr;
	writeDescriptor.descriptorCount = 1;
	writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	writeDescriptor.dstArrayElement = 0;
	writeDescriptor.dstBinding = bindingIndex;
	writeDescriptor.dstSet = m_tempSubpassStates[subpassIndex]->m_descriptorSets[1];

	VkDescriptorImageInfo info;
	info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	info.imageView = view;
	info.sampler = H_NULL_HANDLE;
	writeDescriptor.pImageInfo = &info;

	HLogicalDevice* device = HManager::GetLogicalDevice();

	vkUpdateDescriptorSets(*device, 1, &writeDescriptor, 0, nullptr);
}


//-----------------------------------------------------------------------------------------------
void HMaterial::BindInputAttachment(uint32 name, HephImageView view, uint32 subpassIndex)
{
	uint32 binding = m_tempSubpassStates[subpassIndex]->FindBindPointFor(name);
	BindInputAttachment(view, binding, subpassIndex);
}


//-----------------------------------------------------------------------------------------------
void HMaterial::BindReadAttachment(HephImageView view, uint32 bindingIndex, uint32 subpassIndex)
{
	HephWriteDescriptorSet_T writeDescriptor;
	writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptor.pNext = nullptr;
	writeDescriptor.descriptorCount = 1;
	writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptor.dstArrayElement = 0;
	writeDescriptor.dstBinding = bindingIndex;
	writeDescriptor.dstSet = m_tempSubpassStates[subpassIndex]->m_descriptorSets[1];

	VkDescriptorImageInfo info;
	info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	info.imageView = view;
	info.sampler = HTexture::GetDefaultSampler();
	writeDescriptor.pImageInfo = &info;

	HLogicalDevice* device = HManager::GetLogicalDevice();

	vkUpdateDescriptorSets(*device, 1, &writeDescriptor, 0, nullptr);
}


//-----------------------------------------------------------------------------------------------
void HMaterial::BindReadAttachment(uint32 name, HephImageView view, uint32 subpassIndex)
{
	uint32 binding = m_tempSubpassStates[subpassIndex]->FindBindPointFor(name);
	BindReadAttachment(view, binding, subpassIndex);
}


//-----------------------------------------------------------------------------------------------
void HMaterial::BindState(HCommandBuffer* commandBuffer, uint32 renderPassHandle, uint32 subpassIndex)
{
	auto renderPassStates = m_states.Find(renderPassHandle);
	HMaterialState* thisState = (*renderPassStates)[subpassIndex];
	thisState->Bind(commandBuffer);
}


//-----------------------------------------------------------------------------------------------
void HMaterial::BindTexture(HTexture* texture, uint32 bindingIndex, uint32 subpassIndex /* = 0 */)
{
	HephWriteDescriptorSet_T writeDescriptor = texture->GetWriteDescriptorCopy();
	writeDescriptor.dstBinding = bindingIndex;
	writeDescriptor.dstSet = m_tempSubpassStates[subpassIndex]->m_descriptorSets[1];
	HLogicalDevice* device = HManager::GetLogicalDevice();

	vkUpdateDescriptorSets(*device, 1, &writeDescriptor, 0, nullptr);
}


//-----------------------------------------------------------------------------------------------
void HMaterial::BindTexture(HTexture* texture, const QuString& name, uint32 subpassIndex /* = 0 */)
{
	uint32 binding = m_tempSubpassStates[subpassIndex]->FindBindPointFor(name);
	BindTexture(texture, binding, subpassIndex);
}


//-----------------------------------------------------------------------------------------------
void HMaterial::BindUniformBufferData(const QuString& name, void* data, uint32 dataSize, const QuString& renderPassIndexString)
{
	std::vector<QuString> indexStrings = renderPassIndexString.SplitOnDelimiter('.');
	ASSERT_OR_DIE(indexStrings.size() == 2, "renderPassIndexString must have the following form: renderPassName.subpassName\n");
	uint32 renderPassNameHash = indexStrings[0];
	HRenderPass* renderPass = HManager::GetRenderPassByName(renderPassNameHash);
	auto statesForRenderPass = m_states.Find(renderPassNameHash);
	uint32 subpassIndex = renderPass->RetrieveSubpass(indexStrings[1]).subpassIndex;

	(*statesForRenderPass)[subpassIndex]->BindUniformBufferData(name, data, dataSize);
}


//-----------------------------------------------------------------------------------------------
void HMaterial::BindUniformBufferData(const QuString& name, void* data, uint32 dataSize)
{
	uint32 renderPassCount = m_states.GetSize();
	FOR_COUNT(renderPassIndex, renderPassCount)
	{
		auto subpassStates = m_states.GetValueAtIndex(renderPassIndex);
		uint32 subpassCount = subpassStates->size();
		FOR_COUNT(subpassIndex, subpassCount)
		{
			HMaterialState* state = (*subpassStates)[subpassIndex];
			if (state)
			{
				state->BindUniformBufferData(name, data, dataSize);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void HMaterial::BindTexelBufferData(const QuString& name, void* data, uint32 dataSize, const QuString& renderPassIndexString)
{
	std::vector<QuString> indexStrings = renderPassIndexString.SplitOnDelimiter('.');
	ASSERT_OR_DIE(indexStrings.size() == 2, "renderPassIndexString must have the following form: renderPassName.subpassName\n");
	uint32 renderPassNameHash = indexStrings[0];
	HRenderPass* renderPass = HManager::GetRenderPassByName(renderPassNameHash);
	auto statesForRenderPass = m_states.Find(renderPassNameHash);
	uint32 subpassIndex = renderPass->RetrieveSubpass(indexStrings[1]).subpassIndex;

	(*statesForRenderPass)[subpassIndex]->BindTexelBufferData(name, data, dataSize);
}


//-----------------------------------------------------------------------------------------------
void HMaterial::BindTexelBufferData(const QuString& name, void* data, uint32 dataSize)
{
	uint32 renderPassCount = m_states.GetSize();
	FOR_COUNT(renderPassIndex, renderPassCount)
	{
		auto subpassStates = m_states.GetValueAtIndex(renderPassIndex);
		uint32 subpassCount = subpassStates->size();
		FOR_COUNT(subpassIndex, subpassCount)
		{
			HMaterialState* state = (*subpassStates)[subpassIndex];
			if (state)
			{
				state->BindTexelBufferData(name, data, dataSize);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
static bool ParseMappingNode(const XMLNode& node, const QuString& mappingSrcName, QuString* pOutAssociation)
{
	XML_EXTRACT_ATTRIBUTE(node, srcName);
	if (srcName != mappingSrcName)
	{
		return false;
	}

	XML_EXTRACT_ATTRIBUTE(node, dstMat);
	ASSERT_OR_DIE(pOutAssociation, "Why is this pointer null\n");
	*pOutAssociation = dstMat;

	return true;
}


//-----------------------------------------------------------------------------------------------
STATIC HMaterial HMaterial::FromAssociation(const QuString& assocName, const QuString& mappingSrcName)
{
	QuString assocFilePath = QuString::F("Data/Materials/%s.Association.xml", assocName.GetRaw());

	XMLNode rootNode = XMLUtils::GetRootNode(assocFilePath);
	ASSERT_OR_DIE(QuString(rootNode.getName()) == "Associations", "Root node of association file should be called Associations\n");

	uint32 mappingCount = rootNode.nChildNode();
	bool foundMapping = false;
	QuString association;
	FOR_COUNT(mappingIndex, mappingCount)
	{
		XMLNode mappingNode = rootNode.getChildNode(mappingIndex);
		ASSERT_OR_DIE(QuString(mappingNode.getName()) == "Mapping", "Expected Mapping sub-node\n");

		foundMapping = ParseMappingNode(mappingNode, mappingSrcName, &association);
		if (foundMapping)
		{
			break;
		}
	}

	ASSERT_OR_DIE(foundMapping, "Could not find mapping for association src\n");

	return HMaterial(association);
}
#include "Quantum/Hephaestus/VertexType.h"

#include <vulkan.h>


//-----------------------------------------------------------------------------------------------
STATIC HVertexType HVertexType::s_vertexTypes[H_VERTEX_TYPE_COUNT];


//-----------------------------------------------------------------------------------------------
static uint32 GetStride(EVertexType vertexType)
{
	switch (vertexType)
	{
	case H_VERTEX_TYPE_P:
		return sizeof(HVertexP);
	case H_VERTEX_TYPE_PCT:
		return sizeof(HVertexPCT);
	case H_VERTEX_TYPE_PCTN:
		return sizeof(HVertexPCTN);
	case H_VERTEX_TYPE_PCTNT:
		return sizeof(HVertexPCTNT);
	default:
		return 0;
	}
}


//-----------------------------------------------------------------------------------------------
static void FillAttributes(EVertexType vertexType, uint32* pOutAttribCount, HephVertexInputAttributeDescription* ppOutAttribDesc)
{
	switch (vertexType)
	{
	case H_VERTEX_TYPE_P:
	{
		*pOutAttribCount = 1;
		*ppOutAttribDesc = new VkVertexInputAttributeDescription[*pOutAttribCount];
		HephVertexInputAttributeDescription currentDesc = *ppOutAttribDesc;
		//Position
		currentDesc->binding = 0;
		currentDesc->format = VK_FORMAT_R32G32B32_SFLOAT;
		currentDesc->location = 0;
		currentDesc->offset = offsetof(HVertexP, position);
		break;
	}
	case H_VERTEX_TYPE_PCT:
	{
		*pOutAttribCount = 3;
		*ppOutAttribDesc = new VkVertexInputAttributeDescription[*pOutAttribCount];
		HephVertexInputAttributeDescription currentDesc = *ppOutAttribDesc;
		//Position
		currentDesc->binding = 0;
		currentDesc->format = VK_FORMAT_R32G32B32_SFLOAT;
		currentDesc->location = 0;
		currentDesc->offset = offsetof(HVertexPCT, position);
		currentDesc++;
		//Color
		currentDesc->binding = 0;
		currentDesc->format = VK_FORMAT_R32G32B32A32_SFLOAT; //May need to revisit this.  We'll see how it does
		currentDesc->location = 1;
		currentDesc->offset = offsetof(HVertexPCT, color);
		currentDesc++;
		//UVs
		currentDesc->binding = 0;
		currentDesc->format = VK_FORMAT_R32G32_SFLOAT;
		currentDesc->location = 2;
		currentDesc->offset = offsetof(HVertexPCT, uv);
		break;
	}
	case H_VERTEX_TYPE_PCTN:
	{
		*pOutAttribCount = 4;
		*ppOutAttribDesc = new VkVertexInputAttributeDescription[*pOutAttribCount];
		HephVertexInputAttributeDescription currentDesc = *ppOutAttribDesc;
		//Position
		currentDesc->binding = 0;
		currentDesc->format = VK_FORMAT_R32G32B32_SFLOAT;
		currentDesc->location = 0;
		currentDesc->offset = offsetof(HVertexPCTN, position);
		currentDesc++;
		//Color
		currentDesc->binding = 0;
		currentDesc->format = VK_FORMAT_R32G32B32A32_SFLOAT; //May need to revisit this.  We'll see how it does
		currentDesc->location = 1;
		currentDesc->offset = offsetof(HVertexPCTN, color);
		currentDesc++;
		//UVs
		currentDesc->binding = 0;
		currentDesc->format = VK_FORMAT_R32G32_SFLOAT;
		currentDesc->location = 2;
		currentDesc->offset = offsetof(HVertexPCTN, uv);
		currentDesc++;
		//Normals
		currentDesc->binding = 0;
		currentDesc->format = VK_FORMAT_R32G32B32_SFLOAT;
		currentDesc->location = 3;
		currentDesc->offset = offsetof(HVertexPCTN, normal);
		break;
	}
	case H_VERTEX_TYPE_PCTNT:
	{
		*pOutAttribCount = 5;
		*ppOutAttribDesc = new VkVertexInputAttributeDescription[*pOutAttribCount];
		HephVertexInputAttributeDescription currentDesc = *ppOutAttribDesc;
		//Position
		currentDesc->binding = 0;
		currentDesc->format = VK_FORMAT_R32G32B32_SFLOAT;
		currentDesc->location = 0;
		currentDesc->offset = offsetof(HVertexPCTNT, position);
		currentDesc++;
		//Color
		currentDesc->binding = 0;
		currentDesc->format = VK_FORMAT_R32G32B32A32_SFLOAT; //May need to revisit this.  We'll see how it does
		currentDesc->location = 1;
		currentDesc->offset = offsetof(HVertexPCTNT, color);
		currentDesc++;
		//UVs
		currentDesc->binding = 0;
		currentDesc->format = VK_FORMAT_R32G32_SFLOAT;
		currentDesc->location = 2;
		currentDesc->offset = offsetof(HVertexPCTNT, uv);
		currentDesc++;
		//Normals
		currentDesc->binding = 0;
		currentDesc->format = VK_FORMAT_R32G32B32A32_SFLOAT;
		currentDesc->location = 3;
		currentDesc->offset = offsetof(HVertexPCTNT, normal);
		currentDesc++;
		//Tangents
		currentDesc->binding = 0;
		currentDesc->format = VK_FORMAT_R32G32B32_SFLOAT;
		currentDesc->location = 4;
		currentDesc->offset = offsetof(HVertexPCTNT, tangent);
		currentDesc++;
	}
	}
}


//-----------------------------------------------------------------------------------------------
void HVertexType::Initialize(EVertexType thisType)
{
	m_inputState = new VkPipelineVertexInputStateCreateInfo();
	m_inputState->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	m_inputState->pNext = nullptr;
	m_inputState->flags = 0;
	m_inputState->vertexBindingDescriptionCount = 1;
	m_bindingDesc = new VkVertexInputBindingDescription();
	m_bindingDesc->binding = 0;
	m_bindingDesc->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	m_bindingDesc->stride = GetStride(thisType);
	m_inputState->pVertexBindingDescriptions = m_bindingDesc;

	FillAttributes(thisType, &m_inputState->vertexAttributeDescriptionCount, &m_attribDesc);
	m_inputState->pVertexAttributeDescriptions = m_attribDesc;
}


//-----------------------------------------------------------------------------------------------
void HVertexType::Deinitialize()
{
	SAFE_DELETE(m_inputState);
	SAFE_DELETE(m_bindingDesc);
	SAFE_DELETE_ARRAY(m_attribDesc);
}


//-----------------------------------------------------------------------------------------------
STATIC void HVertexType::InitializeTypes()
{
	for (uint32 i = 0; i < H_VERTEX_TYPE_COUNT; i++)
	{
		s_vertexTypes[i].Initialize((EVertexType)i);
	}
}


//-----------------------------------------------------------------------------------------------
STATIC void HVertexType::DeinitializeTypes()
{
	for (uint32 i = 0; i < H_VERTEX_TYPE_COUNT; i++)
	{
		s_vertexTypes[i].Deinitialize();
	}
}
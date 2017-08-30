#include "Quantum/Hephaestus/Mesh.h"
#include "Quantum/Hephaestus/Manager.h"
#include "Quantum/Hephaestus/LogicalDevice.h"
#include "Quantum/Hephaestus/PhysicalDevice.h"
#include "Quantum/Hephaestus/CommandBuffer.h"
#include "Quantum/Hephaestus/VertexType.h"

#include <vulkan.h>

//-----------------------------------------------------------------------------------------------
void HMesh::SetVertexData(void* data, uint32 dataSize, uint32 numVertices)
{
	VkBufferCreateInfo vertexBufferCreateInfo;
	vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferCreateInfo.pNext = nullptr;
	vertexBufferCreateInfo.flags = 0;	//No sparse binding
	vertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vertexBufferCreateInfo.queueFamilyIndexCount = 0;
	vertexBufferCreateInfo.pQueueFamilyIndices = nullptr;
	vertexBufferCreateInfo.size = dataSize;
	vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;	//Maybe later use a staging buffer and such

	HLogicalDevice* device = HManager::GetLogicalDevice();
	HPhysicalDevice* gpu = HManager::GetPhysicalDevice();

	H_ASSERT(vkCreateBuffer(*device, &vertexBufferCreateInfo, nullptr, &m_vertexBuffer), "Could not create vertex buffer\n");

	VkMemoryRequirements vertexMemoryReq;
	vkGetBufferMemoryRequirements(*device, m_vertexBuffer, &vertexMemoryReq);

	VkDeviceMemory vertexBufferMemory;
	VkMemoryAllocateInfo vertexBufferAllocInfo;
	vertexBufferAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vertexBufferAllocInfo.pNext = nullptr;
	vertexBufferAllocInfo.allocationSize = vertexMemoryReq.size;
	vertexBufferAllocInfo.memoryTypeIndex = gpu->GetMemoryTypeIndex(vertexMemoryReq.memoryTypeBits, H_MEMORY_TYPE_HOST_READABLE);

	H_ASSERT(vkAllocateMemory(*device, &vertexBufferAllocInfo, nullptr, &vertexBufferMemory), "Could not allocate vertex buffer memory\n");

	H_ASSERT(vkBindBufferMemory(*device, m_vertexBuffer, vertexBufferMemory, 0), "Could not back vertex buffer\n");

	void* vertexData;
	H_ASSERT(vkMapMemory(*device, vertexBufferMemory, 0, VK_WHOLE_SIZE, 0, &vertexData), "Could not map vertex data memory\n");

	memcpy(vertexData, data, dataSize);

	vkUnmapMemory(*device, vertexBufferMemory);

	m_numVertices = numVertices;
}


//-----------------------------------------------------------------------------------------------
void HMesh::SetIndexData(void* data, uint32 numIndices, EIndexType type)
{
	uint32 dataSize = numIndices * ((type == H_INDEX_TYPE_UINT16) ? sizeof(uint16) : sizeof(uint32));

	VkBufferCreateInfo indexBufferCreateInfo;
	indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexBufferCreateInfo.pNext = nullptr;
	indexBufferCreateInfo.flags = 0;	//No sparse binding
	indexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	indexBufferCreateInfo.queueFamilyIndexCount = 0;
	indexBufferCreateInfo.pQueueFamilyIndices = nullptr;
	indexBufferCreateInfo.size = dataSize;
	indexBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;	//Maybe later use a staging buffer and such

	HLogicalDevice* device = HManager::GetLogicalDevice();
	HPhysicalDevice* gpu = HManager::GetPhysicalDevice();

	H_ASSERT(vkCreateBuffer(*device, &indexBufferCreateInfo, nullptr, &m_indexBuffer), "Could not create index buffer\n");

	VkMemoryRequirements indexMemoryReq;
	vkGetBufferMemoryRequirements(*device, m_indexBuffer, &indexMemoryReq);

	VkDeviceMemory indexBufferMemory;
	VkMemoryAllocateInfo indexBufferAllocInfo;
	indexBufferAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	indexBufferAllocInfo.pNext = nullptr;
	indexBufferAllocInfo.allocationSize = indexMemoryReq.size;
	indexBufferAllocInfo.memoryTypeIndex = gpu->GetMemoryTypeIndex(indexMemoryReq.memoryTypeBits, H_MEMORY_TYPE_HOST_READABLE);

	H_ASSERT(vkAllocateMemory(*device, &indexBufferAllocInfo, nullptr, &indexBufferMemory), "Could not allocate index buffer memory\n");

	H_ASSERT(vkBindBufferMemory(*device, m_indexBuffer, indexBufferMemory, 0), "Could not back index buffer\n");

	void* indexData;
	H_ASSERT(vkMapMemory(*device, indexBufferMemory, 0, VK_WHOLE_SIZE, 0, &indexData), "Could not map vertex data memory\n");

	memcpy(indexData, data, dataSize);

	vkUnmapMemory(*device, indexBufferMemory);

	m_numIndices = numIndices;
	m_indexType = type;
}


//-----------------------------------------------------------------------------------------------
void HMesh::Draw(HCommandBuffer* commandBuffer)
{
	ASSERT_OR_DIE(m_vertexBuffer != H_NULL_HANDLE, "Cannot draw without vertex data\n");
	commandBuffer->BindVertexBuffer(m_vertexBuffer);

	if (m_indexBuffer == H_NULL_HANDLE)
	{
		commandBuffer->Draw(m_numVertices);
	}
	else
	{
		commandBuffer->BindIndexBuffer(m_indexBuffer, m_indexType);
		commandBuffer->DrawIndexed(m_numIndices);
	}
}


//-----------------------------------------------------------------------------------------------
STATIC HMesh HMesh::GetSkyboxMesh()
{
	HVertexPCT verts[8];
	verts[0].position = float3(-.5f, .5f, -.5f);
	verts[1].position = float3(-.5f, .5f, .5f);
	verts[2].position = float3(-.5f, -.5f, -.5f);
	verts[3].position = float3(-.5f, -.5f, .5f);
	verts[4].position = float3(.5f, .5f, -.5f);
	verts[5].position = float3(.5f, .5f, .5f);
	verts[6].position = float3(.5f, -.5f, -.5f);
	verts[7].position = float3(.5f, -.5f, .5f);

	HMesh result;
	result.SetVertexData(verts, sizeof(verts), ARRAY_LENGTH(verts));

	uint16 indices[] =
	{
		0, 2, 1,
		1, 2, 3,

		0, 1, 4,
		4, 1, 5,

		1, 3, 5,
		5, 3, 7,

		5, 7, 4,
		4, 7, 6,

		3, 2, 7,
		7, 2, 6,

		4, 6, 0,
		0, 6, 2
	};

	result.SetIndexData(indices, ARRAY_LENGTH(indices), H_INDEX_TYPE_UINT16);
	
	return result;
}
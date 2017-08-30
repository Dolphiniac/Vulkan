#include "Quantum/Hephaestus/Texture.h"
#include "Quantum/Core/String.h"
#include "Quantum/Hephaestus/LogicalDevice.h"
#include "Quantum/Hephaestus/Manager.h"
#include "Quantum/Hephaestus/PhysicalDevice.h"
#include "Quantum/Hephaestus/Queue.h"
#include "Quantum/Hephaestus/CommandBuffer.h"

#include <vulkan.h>

#define STBI_HEADER_FILE_ONLY
#include "ThirdParty/stb/stb_image.c"


//-----------------------------------------------------------------------------------------------
STATIC std::map<uint32, HTexture*> HTexture::s_textures;
STATIC HephSampler HTexture::s_defaultSampler = H_NULL_HANDLE;


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HTexture::HTexture(const QuString& filepath, ETextureType type)
	: m_type(type)
{
	int32 pixelHeight;
	int32 pixelWidth;
	int32 numComponentsInImage;
	int32 numComponentsRequested = 4;
	uint8* imageData = stbi_load(filepath.GetRaw(), &pixelWidth, &pixelHeight, &numComponentsInImage, numComponentsRequested);	//Must always have 4 components for supported image formats

	uint32 bufferSize = pixelWidth * pixelHeight * numComponentsRequested;

	VkBufferCreateInfo stagingBufferCreateInfo;
	stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferCreateInfo.pNext = nullptr;
	stagingBufferCreateInfo.flags = 0;
	stagingBufferCreateInfo.size = bufferSize;
	stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	stagingBufferCreateInfo.queueFamilyIndexCount = 0;
	stagingBufferCreateInfo.pQueueFamilyIndices = nullptr;

	HLogicalDevice* device = HManager::GetLogicalDevice();
	HPhysicalDevice* gpu = HManager::GetPhysicalDevice();
	
	VkBuffer stagingBuffer;
	H_ASSERT(vkCreateBuffer(*device, &stagingBufferCreateInfo, nullptr, &stagingBuffer), "Could not create texture staging buffer\n");

	VkMemoryRequirements stagingMemoryReq;
	vkGetBufferMemoryRequirements(*device, stagingBuffer, &stagingMemoryReq);

	VkDeviceMemory stagingBufferMemory;
	VkMemoryAllocateInfo stagingBufferAllocInfo;
	stagingBufferAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	stagingBufferAllocInfo.pNext = nullptr;
	stagingBufferAllocInfo.allocationSize = stagingMemoryReq.size;
	stagingBufferAllocInfo.memoryTypeIndex = gpu->GetMemoryTypeIndex(stagingMemoryReq.memoryTypeBits, H_MEMORY_TYPE_HOST_READABLE);

	H_ASSERT(vkAllocateMemory(*device, &stagingBufferAllocInfo, nullptr, &stagingBufferMemory), "Could not allocate staging buffer memory\n");
	H_ASSERT(vkBindBufferMemory(*device, stagingBuffer, stagingBufferMemory, 0), "Could not back staging buffer\n");

	void* bufferData;
	H_ASSERT(vkMapMemory(*device, stagingBufferMemory, 0, VK_WHOLE_SIZE, 0, &bufferData), "Could not map staging buffer memory\n");

	memcpy(bufferData, imageData, bufferSize);
	vkUnmapMemory(*device, stagingBufferMemory);

	VkImageCreateInfo textureCreateInfo;
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.pNext = nullptr;
	textureCreateInfo.flags = 0;
	textureCreateInfo.arrayLayers = 1;
	textureCreateInfo.extent.depth = 1;
	textureCreateInfo.extent.width = pixelWidth;
	textureCreateInfo.extent.height = pixelHeight;
	textureCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	textureCreateInfo.mipLevels = 1;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	textureCreateInfo.queueFamilyIndexCount = 0;
	textureCreateInfo.pQueueFamilyIndices = nullptr;

	if (type == H_TEXTURE_TYPE_CUBE_MAP)
	{
		textureCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		textureCreateInfo.arrayLayers = 6;
		textureCreateInfo.extent.width = pixelWidth / 4;
		textureCreateInfo.extent.height = pixelHeight / 3;
	}

	H_ASSERT(vkCreateImage(*device, &textureCreateInfo, nullptr, &m_image), "Could not create texture image\n");

	VkMemoryRequirements textureMemoryReq;
	vkGetImageMemoryRequirements(*device, m_image, &textureMemoryReq);

	VkMemoryAllocateInfo textureMemoryAllocInfo;
	textureMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	textureMemoryAllocInfo.pNext = nullptr;
	textureMemoryAllocInfo.allocationSize = textureMemoryReq.size;
	textureMemoryAllocInfo.memoryTypeIndex = gpu->GetMemoryTypeIndex(textureMemoryReq.memoryTypeBits, H_MEMORY_TYPE_DEVICE_LOCAL);

	VkDeviceMemory textureMemory;
	H_ASSERT(vkAllocateMemory(*device, &textureMemoryAllocInfo, nullptr, &textureMemory), "Could not allocate texture memory\n");
	H_ASSERT(vkBindImageMemory(*device, m_image, textureMemory, 0), "Could not back texture image\n");

	HQueue* transferQueue = HManager::GetTransferQueue();

	HCommandBufferImmediate* commandBuffer = transferQueue->RetrieveImmediateCommandBuffer();

	commandBuffer->InsertImagePipelineBarrier(m_image, H_IMAGE_ASPECT_COLOR, H_IMAGE_LAYOUT_UNDEFINED, H_IMAGE_LAYOUT_TRANSFER_WRITE, H_PIPELINE_STAGE_HOST, H_PIPELINE_STAGE_TRANSFER_WRITE);

	switch (type)
	{
	case H_TEXTURE_TYPE_2D:
		commandBuffer->CopyBufferToImage(stagingBuffer, m_image, pixelWidth, pixelHeight);
		break;
	case H_TEXTURE_TYPE_CUBE_MAP:
		commandBuffer->CopyBufferToImageCube(stagingBuffer, m_image, pixelWidth, pixelHeight);
		break;
	default:
		ERROR_AND_DIE("Unsupported texture type\n");
	}

	commandBuffer->InsertImagePipelineBarrier(m_image, H_IMAGE_ASPECT_COLOR, H_IMAGE_LAYOUT_TRANSFER_WRITE, H_IMAGE_LAYOUT_SHADER_READ_OPTIMAL, H_PIPELINE_STAGE_TRANSFER_WRITE, H_PIPELINE_STAGE_FRAGMENT_SHADER_READ);

	VkImageViewCreateInfo viewCreateInfo;
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.pNext = nullptr;
	viewCreateInfo.flags = 0;
	viewCreateInfo.components = {};	//Identity
	viewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	viewCreateInfo.image = m_image;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	viewCreateInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

	if (type == H_TEXTURE_TYPE_CUBE_MAP)
	{
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	}

	H_ASSERT(vkCreateImageView(*device, &viewCreateInfo, nullptr, &m_view), "Could not create texture view\n");

	m_writeDescriptorSet = new VkWriteDescriptorSet();
	m_writeDescriptorSet->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	m_writeDescriptorSet->pNext = nullptr;
	m_writeDescriptorSet->descriptorCount = 1;
	m_writeDescriptorSet->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	m_writeDescriptorSet->dstArrayElement = 0;
	m_writeDescriptorSet->dstBinding = H_INVALID;
	m_writeDescriptorSet->dstSet = H_NULL_HANDLE;
	m_writeDescriptorSet->pBufferInfo = nullptr;
	m_writeDescriptorSet->pTexelBufferView = nullptr;
	VkDescriptorImageInfo* imageInfo = new VkDescriptorImageInfo();
	imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo->imageView = m_view;

	VkSamplerCreateInfo samplerCreateInfo;
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = nullptr;
	samplerCreateInfo.flags = 0;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.anisotropyEnable = VK_FALSE;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.maxAnisotropy = 0.f;
	samplerCreateInfo.maxLod = 0.f;
	samplerCreateInfo.minLod = 0.f;
	samplerCreateInfo.mipLodBias = 0.f;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

	H_ASSERT(vkCreateSampler(*device, &samplerCreateInfo, nullptr, &m_sampler), "Could not create sampler\n");
	imageInfo->sampler = m_sampler;
	m_writeDescriptorSet->pImageInfo = imageInfo;
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
STATIC HTexture* HTexture::CreateOrGetTexture(const QuString& filepath, ETextureType type /* = H_TEXTURE_TYPE_2D */)
{
	auto texIter = s_textures.find(filepath);

	if (texIter != s_textures.end())
	{
		ASSERT_OR_DIE(texIter->second->m_type == type, "Incompatible texture type\n");
		return texIter->second;
	}

	HTexture* result = new HTexture(filepath, type);
	s_textures.insert(std::make_pair(filepath, result));

	return result;
}


//-----------------------------------------------------------------------------------------------
STATIC HephSampler HTexture::GetDefaultSampler()
{
	if (s_defaultSampler == H_NULL_HANDLE)
	{
		VkSamplerCreateInfo samplerCreateInfo;
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.pNext = nullptr;
		samplerCreateInfo.flags = 0;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.anisotropyEnable = VK_FALSE;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.maxAnisotropy = 0.f;
		samplerCreateInfo.maxLod = 1.f;
		samplerCreateInfo.minLod = 1.f;
		samplerCreateInfo.mipLodBias = 0.f;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

		HLogicalDevice* device = HManager::GetLogicalDevice();
		H_ASSERT(vkCreateSampler(*device, &samplerCreateInfo, nullptr, &s_defaultSampler), "Could not create default sampler\n");
	}

	return s_defaultSampler;
}


//-----------------------------------------------------------------------------------------------
HephWriteDescriptorSet_T HTexture::GetWriteDescriptorCopy() const
{
	HephWriteDescriptorSet_T result;
	memcpy(&result, m_writeDescriptorSet, sizeof(result));
	return result;
}
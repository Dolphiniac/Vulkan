#include "Quantum/Hephaestus/BufferDescriptor.h"
#include "Quantum/Core/String.h"
#include "Quantum/Hephaestus/Manager.h"
#include "Quantum/Hephaestus/LogicalDevice.h"
#include "Quantum/Hephaestus/PhysicalDevice.h"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <vulkan.h>
#include <vector>


//-----------------------------------------------------------------------------------------------
//START UNIFORM BUFFER DECLARATIONS
//-----------------------------------------------------------------------------------------------


H_UNIFORM_DESCRIPTOR_CPP_BEGIN(Time)
H_UNIFORM_DESCRIPTOR_CPP_DECLARE_MEMBER(Time, float, time)
H_UNIFORM_DESCRIPTOR_CPP_END(Time)

H_UNIFORM_DESCRIPTOR_CPP_BEGIN(GlobalMatrices)
H_UNIFORM_DESCRIPTOR_CPP_DECLARE_MEMBER(GlobalMatrices, Matrix44, View)
H_UNIFORM_DESCRIPTOR_CPP_DECLARE_MEMBER(GlobalMatrices, Matrix44, InvView)
H_UNIFORM_DESCRIPTOR_CPP_DECLARE_MEMBER(GlobalMatrices, Matrix44, Projection)
H_UNIFORM_DESCRIPTOR_CPP_DECLARE_MEMBER(GlobalMatrices, Matrix44, InvProjection)
H_UNIFORM_DESCRIPTOR_CPP_END(GlobalMatrices)


//-----------------------------------------------------------------------------------------------
//END UNIFORM BUFFER DECLARATIONS
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
uint32 g_bindingIndex = START_BINDING_INDEX;
uint32 g_numberGlobalDescriptors = 0;


//-----------------------------------------------------------------------------------------------
STATIC std::map<uint32, HUniform*>* HUniform::s_globalUniforms = nullptr;
STATIC bool HUniform::s_hasBeenInitialized = false;
STATIC HephDescriptorSet HUniform::s_descriptorSet = H_NULL_HANDLE;
STATIC HephDescriptorSetLayout HUniform::s_descriptorSetLayout = H_NULL_HANDLE;
STATIC HephDescriptorPool HUniform::s_descriptorPool = H_NULL_HANDLE;


//-----------------------------------------------------------------------------------------------
STATIC QuString* HGlobalShaderIncludeBuilder::s_globalShader = nullptr;


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
HUniform::HUniform(uint32 bindingIndex, uint32 bufferSize, void* cpuBuffer)
	: m_cpuBufferSize(bufferSize)
	, m_cpuBufferData(cpuBuffer)
{
}


//-----------------------------------------------------------------------------------------------
void HUniform::InitializeBufferAndMemory()
{
	//Here, we create a buffer, and the memory associated with it, then bind them together
	VkBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;
	bufferCreateInfo.size = m_cpuBufferSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	HLogicalDevice* device = HManager::GetLogicalDevice();

	H_ASSERT(vkCreateBuffer(*device, &bufferCreateInfo, nullptr, &m_buffer), "Could not create uniform buffer\n");

	VkMemoryRequirements bufferMemoryRequirements;
	vkGetBufferMemoryRequirements(*device, m_buffer, &bufferMemoryRequirements);

	VkMemoryAllocateInfo memoryAllocInfo;
	memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.pNext = nullptr;
	memoryAllocInfo.allocationSize = bufferMemoryRequirements.size;

	HPhysicalDevice* gpu = HManager::GetPhysicalDevice();
	memoryAllocInfo.memoryTypeIndex = gpu->GetMemoryTypeIndex(bufferMemoryRequirements.memoryTypeBits, H_MEMORY_TYPE_HOST_READABLE);

	H_ASSERT(vkAllocateMemory(*device, &memoryAllocInfo, nullptr, &m_memory), "Could not allocate memory\n");

	vkBindBufferMemory(*device, m_buffer, m_memory, 0);
}


//-----------------------------------------------------------------------------------------------
HUniform::~HUniform()
{
	HLogicalDevice* device = HManager::GetLogicalDevice();

	vkDestroyBuffer(*device, m_buffer, nullptr);
	vkFreeMemory(*device, m_memory, nullptr);
}


//-----------------------------------------------------------------------------------------------
HGlobalShaderIncludeBuilder::HGlobalShaderIncludeBuilder(const char* name, bool isStartOfStruct)
{
	if (!s_globalShader)
	{
		s_globalShader = new QuString("#version 450 core\n\n"
			"mat4 InverseOrthonormal(mat4 inMat)\n"
			"{\n"
			"	mat3 upper = mat3(inMat);\n"
			"	upper = transpose(upper);\n"
			"	vec4 row0 = vec4(upper[0], 0.f);\n"
			"	vec4 row1 = vec4(upper[1], 0.f);\n"
			"	vec4 row2 = vec4(upper[2], 0.f);\n"
			"	vec4 row3 = inMat[3];\n"
			"	row3 = -row3;\n"
			"	row3[3] = 1.f;\n"
			"	return mat4(row0, row1, row2, row3);\n"
			"}\n\n");
	}

	if (isStartOfStruct)
	{
		//Start of uniform buffer struct
		//layout (binding = currBindingIndex) uniform name {
		*s_globalShader += "layout (binding = ";
		*s_globalShader += g_bindingIndex;
		*s_globalShader += ") uniform H";
		*s_globalShader += name;
		*s_globalShader += "\n{\n";
	}
	else
	{
		//End of uniform buffer struct
		//} name;
		*s_globalShader += "} ";
		*s_globalShader += name;
		*s_globalShader += ";\n\n";
	}
}


static bool GetShaderTypeNameForType(const QuString& typeName, char* outShaderTypeName);
//-----------------------------------------------------------------------------------------------
HGlobalShaderIncludeBuilder::HGlobalShaderIncludeBuilder(const char* typeName, const char* name)
{
	ASSERT_OR_DIE(s_globalShader, "Cannot declare member before beginning of struct\n");

	char shaderTypeName[32];
	ASSERT_OR_DIE(GetShaderTypeNameForType(typeName, shaderTypeName), "Cannot recognize type\n");

	//Declaration of member variable
	//typename name;
	*s_globalShader += shaderTypeName;
	*s_globalShader += " ";
	*s_globalShader += name;
	*s_globalShader += ";\n";
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
void HUniform::Push() const
{
	if (!s_hasBeenInitialized)
	{
		InitializeDescriptorSet();
	}

	HLogicalDevice* device = HManager::GetLogicalDevice();

	void* data;
	H_ASSERT(vkMapMemory(*device, m_memory, 0, VK_WHOLE_SIZE, 0, &data), "Could not map device memory\n");

	memcpy(data, m_cpuBufferData, m_cpuBufferSize);

	vkUnmapMemory(*device, m_memory);
}


//-----------------------------------------------------------------------------------------------
STATIC HUniform* HUniform::AddUniformBuffer(uint32 bufferSize, void* cpuBuffer)
{
	if (!s_globalUniforms)
	{
		s_globalUniforms = new std::map<uint32, HUniform*>();
	}
	HUniform* result = new HUniform(g_bindingIndex, bufferSize, cpuBuffer);
	s_globalUniforms->insert(std::make_pair(g_bindingIndex, result));
	g_bindingIndex++;
	g_numberGlobalDescriptors++;

	return result;
}


//-----------------------------------------------------------------------------------------------
void HUniform::GetDescriptorSetAndLayout(HephDescriptorSetLayout* pOutLayout, HephDescriptorSet* pOutSet)
{
	if (!s_hasBeenInitialized)
	{
		InitializeDescriptorSet();
	}

	if (pOutLayout)
	{
		*pOutLayout = s_descriptorSetLayout;
	}
	if (pOutSet)
	{
		*pOutSet = s_descriptorSet;
	}
}


//-----------------------------------------------------------------------------------------------
STATIC void HUniform::InitializeDescriptorSet()
{
	//-----------------------------------------------------------------------------------------------
	//This one's a doozy.  If we want our uniform buffers to actually function, we have to create
	//descriptors for them.  Since these are all in one place, it's convenient to make a single
	//descriptor set and layout that encompasses them all.  But at the graphics pipeline level,
	//We'll need to allow for new descriptor sets
	//-----------------------------------------------------------------------------------------------
	s_hasBeenInitialized = true;
	HLogicalDevice* device = HManager::GetLogicalDevice();

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo;
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.pNext = nullptr;
	layoutCreateInfo.flags = 0;
	uint32 descriptorCount = s_globalUniforms->size();
	layoutCreateInfo.bindingCount = descriptorCount;

	std::vector<HephDescriptorSetLayoutBinding_T> bindings;
	bindings.resize(descriptorCount);
	uint32 bufferIndex = 0;
	for (auto bufferIter = s_globalUniforms->begin(); bufferIter != s_globalUniforms->end(); bufferIter++)
	{
		HUniform* currentUniform = bufferIter->second;

		currentUniform->InitializeBufferAndMemory();

		HephDescriptorSetLayoutBinding_T* binding = &bindings[bufferIndex];
		binding->stageFlags = VK_SHADER_STAGE_ALL;
		binding->descriptorCount = 1;
		binding->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		binding->pImmutableSamplers = nullptr;
		binding->binding = bufferIter->first;
		bufferIndex++;
	}

	layoutCreateInfo.pBindings = bindings.data();

	H_ASSERT(vkCreateDescriptorSetLayout(*device, &layoutCreateInfo, nullptr, &s_descriptorSetLayout), "Could not create descriptor set layout\n");

	VkDescriptorPoolCreateInfo poolCreateInfo;
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.pNext = nullptr;
	poolCreateInfo.flags = 0;
	poolCreateInfo.maxSets = 1;
	poolCreateInfo.poolSizeCount = 1;

	VkDescriptorPoolSize poolSize;
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = descriptorCount;

	poolCreateInfo.pPoolSizes = &poolSize;

	H_ASSERT(vkCreateDescriptorPool(*device, &poolCreateInfo, nullptr, &s_descriptorPool), "Could not create uniform buffer descriptor pool\n");

	VkDescriptorSetAllocateInfo setAllocInfo;
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.pNext = nullptr;
	setAllocInfo.pSetLayouts = &s_descriptorSetLayout;
	setAllocInfo.descriptorSetCount = 1;
	setAllocInfo.descriptorPool = s_descriptorPool;

	H_ASSERT(vkAllocateDescriptorSets(*device, &setAllocInfo, &s_descriptorSet), "Could not allocate uniform buffer descriptor set\n");

	std::vector<HephWriteDescriptorSet_T> writeDescriptorSets;
	writeDescriptorSets.resize(descriptorCount);
	std::vector<HephDescriptorBufferInfo_T> bufferInfos;
	bufferInfos.resize(descriptorCount);

	bufferIndex = 0;
	for (auto bufferIter = s_globalUniforms->begin(); bufferIter != s_globalUniforms->end(); bufferIter++)
	{
		HephWriteDescriptorSet_T* currentWrite = &writeDescriptorSets[bufferIndex];
		currentWrite->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		currentWrite->pNext = nullptr;
		currentWrite->pImageInfo = nullptr;
		currentWrite->pTexelBufferView = nullptr;
		currentWrite->dstSet = s_descriptorSet;
		currentWrite->descriptorCount = 1;
		currentWrite->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		currentWrite->dstArrayElement = 0;
		currentWrite->dstBinding = bufferIter->first;

		HephDescriptorBufferInfo_T* currentBufferInfo = &bufferInfos[bufferIndex];
		currentBufferInfo->offset = 0;
		currentBufferInfo->range = VK_WHOLE_SIZE;
		currentBufferInfo->buffer = bufferIter->second->m_buffer;

		currentWrite->pBufferInfo = currentBufferInfo;
		bufferIndex++;
	}
	vkUpdateDescriptorSets(*device, descriptorCount, writeDescriptorSets.data(), 0, nullptr);
}


//-----------------------------------------------------------------------------------------------
static bool GetShaderTypeNameForType(const QuString& typeName, char* outShaderTypeName)
{
	if (typeName == "float")
	{
		strcpy_s(outShaderTypeName, 6, "float");
	}
	else if (typeName == "int32")
	{
		strcpy_s(outShaderTypeName, 4, "int");
	}
	else if (typeName == "uint32")
	{
		strcpy_s(outShaderTypeName, 5, "uint");
	}
	else if (typeName == "QuVector2" || typeName == "float2")
	{
		strcpy_s(outShaderTypeName, 5, "vec2");
	}
	else if (typeName == "QuVector3" || typeName == "float3")
	{
		strcpy_s(outShaderTypeName, 5, "vec3");
	}
	else if (typeName == "QuVector4" || typeName == "float4")
	{
		strcpy_s(outShaderTypeName, 5, "vec4");
	}
	else if (typeName == "Matrix44")
	{
		strcpy_s(outShaderTypeName, 24, "layout (row_major) mat4");
	}
	else
	{
		return false;
	}

	return true;
}
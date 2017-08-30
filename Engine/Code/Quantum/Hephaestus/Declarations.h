#pragma once

#include "Engine/Core/ErrorWarningAssert.hpp"

#define H_DECLARE_VK_HANDLE(name) typedef struct Vk##name##_T* Heph##name;
#define H_DECLARE_VK_NONDISPATCHABLE_HANDLE(name) typedef uint64 Heph##name;
#define H_DECLARE_VK_STRUCT(name) typedef struct Vk##name Heph##name##_T; typedef Heph##name##_T* Heph##name;
#define H_DECLARE_VK_UNION(name) typedef union Vk##name Heph##name##_T; typedef Heph##name##_T* Heph##name;
#define H_DECLARE_VK_ENUM(name) typedef enum Vk##name Heph##name;
#define H_DECLARE_WIN32_HANDLE(name) typedef struct name##__* Heph##name;

#define H_INVALID ~0U
#define H_INVALID_INDEX -1
#define H_NULL_HANDLE 0

#define H_SUCCESS 0

#define H_ASSERT(result, text) ASSERT_OR_DIE(result == H_SUCCESS, text);

typedef uint32 HFlags;


//-----------------------------------------------------------------------------------------------
H_DECLARE_VK_HANDLE(Instance)
H_DECLARE_VK_HANDLE(PhysicalDevice)
H_DECLARE_VK_HANDLE(Device)
H_DECLARE_VK_HANDLE(Queue)
H_DECLARE_VK_HANDLE(CommandBuffer)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(SurfaceKHR)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(SwapchainKHR)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(CommandPool)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(ShaderModule)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(Pipeline)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(PipelineCache)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(Buffer)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(DeviceMemory)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(DescriptorSet)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(DescriptorSetLayout)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(DescriptorPool)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(PipelineLayout)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(RenderPass)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(Fence)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(Semaphore)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(Framebuffer)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(Image)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(ImageView)
H_DECLARE_VK_NONDISPATCHABLE_HANDLE(Sampler)

H_DECLARE_VK_STRUCT(PhysicalDeviceFeatures)
H_DECLARE_VK_STRUCT(PhysicalDeviceMemoryProperties)
H_DECLARE_VK_STRUCT(QueueFamilyProperties)
H_DECLARE_VK_STRUCT(DeviceQueueCreateInfo)
H_DECLARE_VK_STRUCT(SwapchainCreateInfoKHR)
H_DECLARE_VK_STRUCT(SurfaceCapabilitiesKHR)
H_DECLARE_VK_STRUCT(PipelineShaderStageCreateInfo)
H_DECLARE_VK_STRUCT(GraphicsPipelineCreateInfo)
H_DECLARE_VK_STRUCT(ComputePipelineCreateInfo)
H_DECLARE_VK_STRUCT(PipelineMultisampleStateCreateInfo)
H_DECLARE_VK_STRUCT(PipelineColorBlendStateCreateInfo)
H_DECLARE_VK_STRUCT(PipelineColorBlendAttachmentState)
H_DECLARE_VK_STRUCT(PipelineDepthStencilStateCreateInfo)
H_DECLARE_VK_STRUCT(PipelineTessellationStateCreateInfo)
H_DECLARE_VK_STRUCT(PipelineViewportStateCreateInfo)
H_DECLARE_VK_STRUCT(PipelineInputAssemblyStateCreateInfo)
H_DECLARE_VK_STRUCT(PipelineRasterizationStateCreateInfo)
H_DECLARE_VK_STRUCT(PipelineVertexInputStateCreateInfo)
H_DECLARE_VK_STRUCT(PipelineDynamicStateCreateInfo)
H_DECLARE_VK_STRUCT(VertexInputBindingDescription)
H_DECLARE_VK_STRUCT(VertexInputAttributeDescription)
H_DECLARE_VK_STRUCT(DescriptorSetLayoutBinding)
H_DECLARE_VK_STRUCT(WriteDescriptorSet)
H_DECLARE_VK_STRUCT(DescriptorBufferInfo)
H_DECLARE_VK_STRUCT(RenderPassCreateInfo)
H_DECLARE_VK_STRUCT(RenderPassBeginInfo)
H_DECLARE_VK_STRUCT(CommandBufferInheritanceInfo)
H_DECLARE_VK_STRUCT(Viewport)
H_DECLARE_VK_STRUCT(Rect2D)
H_DECLARE_VK_STRUCT(SubmitInfo)
H_DECLARE_VK_STRUCT(PresentInfoKHR)
H_DECLARE_VK_STRUCT(DescriptorImageInfo)
H_DECLARE_VK_STRUCT(StencilOpState)
H_DECLARE_VK_STRUCT(AttachmentDescription)
H_DECLARE_VK_STRUCT(Extent3D)
H_DECLARE_VK_STRUCT(SubpassDescription)
H_DECLARE_VK_STRUCT(AttachmentReference)
H_DECLARE_VK_STRUCT(SubpassDependency)

H_DECLARE_VK_UNION(ClearValue)

H_DECLARE_VK_ENUM(PresentModeKHR)
H_DECLARE_VK_ENUM(DynamicState)
H_DECLARE_VK_ENUM(ImageUsageFlagBits)
H_DECLARE_VK_ENUM(Format)

H_DECLARE_WIN32_HANDLE(HINSTANCE)
H_DECLARE_WIN32_HANDLE(HWND)


//-----------------------------------------------------------------------------------------------
//Struct that holds uninitialized device queues.  When passed to a device creation function,
//the queues are initialized via the same array.  Therefore, the queues should be passed in by
//address to pre-existing destination queue objects
struct HQueueTransmitter
{
	class HQueue** ppQueueInitializers = nullptr;
	uint32 numQueueInitializers = H_INVALID;
};


//-----------------------------------------------------------------------------------------------
enum HQueueCapabilityFlagBits
{
	H_QUEUE_CAPABILITY_GRAPHICS_BIT = 0b1,
	H_QUEUE_CAPABILITY_COMPUTE_BIT = 0b10,
	H_QUEUE_CAPABILITY_TRANSFER_BIT = 0b100,
	H_QUEUE_CAPABILITY_SPARSE_BINDING_BIT = 0b1000,
	H_QUEUE_CAPABILITY_BIT_COUNT = 4,
	H_QUEUE_CAPABILITY_ALL_BUT_SPARSE = 0b111,
	H_QUEUE_CAPABILITY_ALL = 0b1111
};
typedef HFlags HQueueCapabilityFlags;


//-----------------------------------------------------------------------------------------------
enum EPresentationMode
{
	H_PRESENTATION_MODE_IMMEDIATE,
	H_PRESENTATION_MODE_DOUBLE_BUFFER,
	H_PRESENTATION_MODE_TRIPLE_BUFFER
};


//-----------------------------------------------------------------------------------------------
enum HShaderTypeBits
{
	H_SHADER_TYPE_VERTEX_BIT = 0b1,
	H_SHADER_TYPE_FRAGMENT_BIT = 0b10,
	H_SHADER_TYPE_TESSELLATION_CONTROL_BIT = 0b100,
	H_SHADER_TYPE_TESSELLATION_EVALUATION_BIT = 0b1000,
	H_SHADER_TYPE_GEOMETRY_BIT = 0b1'0000,
	H_SHADER_TYPE_COMPUTE_BIT = 0b10'0000,
	H_SHADER_TYPE_VERTEX_AND_FRAGMENT_BITS = 0b11
};
typedef HFlags HShaderTypes;


//-----------------------------------------------------------------------------------------------
enum EVertexType
{
	H_VERTEX_TYPE_P,
	H_VERTEX_TYPE_PCT,
	H_VERTEX_TYPE_PCTN,
	H_VERTEX_TYPE_PCTNT,
	H_VERTEX_TYPE_COUNT
};


//-----------------------------------------------------------------------------------------------
enum EIndexType
{
	H_INDEX_TYPE_UINT16,
	H_INDEX_TYPE_UINT32
};


//-----------------------------------------------------------------------------------------------
enum EUniformDescriptorType
{
	H_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	H_DESCRIPTOR_TYPE_SHADER_STORAGE_BUFFER,
	H_DESCRIPTOR_TYPE_SAMPLER2D,
	H_DESCRIPTOR_TYPE_TEXTURE2D,
	H_DESCRIPTOR_TYPE_SAMPLER,
	H_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
	H_DESCRIPTOR_TYPE_TEXEL_BUFFER,
	H_DESCRIPTOR_TYPE_INVALID
};


//-----------------------------------------------------------------------------------------------
enum EMemoryType
{
	H_MEMORY_TYPE_HOST_READABLE,
	H_MEMORY_TYPE_DEVICE_LOCAL
};


//-----------------------------------------------------------------------------------------------
enum EImageAspect
{
	H_IMAGE_ASPECT_COLOR
};


//-----------------------------------------------------------------------------------------------
enum EImageLayout
{
	H_IMAGE_LAYOUT_UNDEFINED,
	H_IMAGE_LAYOUT_SHADER_READ_OPTIMAL,
	H_IMAGE_LAYOUT_TRANSFER_WRITE
};


//-----------------------------------------------------------------------------------------------
enum ETextureType
{
	H_TEXTURE_TYPE_2D,
	H_TEXTURE_TYPE_CUBE_MAP
};


//-----------------------------------------------------------------------------------------------
enum EPipelineStage
{
	H_PIPELINE_STAGE_TRANSFER_WRITE,
	H_PIPELINE_STAGE_FRAGMENT_SHADER_READ,
	H_PIPELINE_STAGE_HOST,
	H_PIPELINE_STAGE_DEPTH_STENCIL_BIT
};
namespace Hephaestus
{
	uint32 TranslateStage(EPipelineStage pipelineStage);
}


//-----------------------------------------------------------------------------------------------
#define H_VALIDATION_LAYER_NAME "VK_LAYER_LUNARG_standard_validation"
#include "Quantum/Hephaestus/Declarations.h"

#include <vulkan.h>


//-----------------------------------------------------------------------------------------------
uint32 Hephaestus::TranslateStage(EPipelineStage pipelineStage)
{
	switch (pipelineStage)
	{
	case H_PIPELINE_STAGE_TRANSFER_WRITE:
		return VK_PIPELINE_STAGE_TRANSFER_BIT;
	case H_PIPELINE_STAGE_FRAGMENT_SHADER_READ:
		return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	case H_PIPELINE_STAGE_HOST:
		return VK_PIPELINE_STAGE_HOST_BIT;
	case H_PIPELINE_STAGE_DEPTH_STENCIL_BIT:
		return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
}
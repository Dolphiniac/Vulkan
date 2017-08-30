#pragma once


//-----------------------------------------------------------------------------------------------
namespace Vulkan
{
	bool CompileGLSLToSPIRV(const char* glslCodePath, uint32*& outSpirVBuffer, uint32& outSize);
}
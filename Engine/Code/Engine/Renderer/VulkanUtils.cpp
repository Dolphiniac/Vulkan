#include "Engine/Renderer/VulkanUtils.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Quantum/Core/String.h"
#include "Engine/Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
bool Vulkan::CompileGLSLToSPIRV(const char* glslCodePath, uint32*& outSpirVBuffer, uint32& outSize)
{
	std::vector<char> spirVBuffer;
	QuString command = QuString::F("glslangvalidator -V %s -o %s.spv", glslCodePath, glslCodePath);

	int error = system(command.GetRaw());

	if (error)
	{
		return false;
	}
	QuString filepath = QuString::F("%s.spv", glslCodePath);
	bool loadSucceeded = LoadBinaryFileToBuffer(filepath.GetRaw(), spirVBuffer);
	if (!loadSucceeded)
	{
		return false;
	}

	outSize = spirVBuffer.size();

	outSpirVBuffer = new uint32[outSize / 4];
	memcpy(outSpirVBuffer, &spirVBuffer[0], outSize);

	return true;
}
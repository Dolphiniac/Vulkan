#pragma once


//-----------------------------------------------------------------------------------------------
namespace Memory
{
	template<typename... Args>
	char* GetVAListFromTemplateArgs(Args... args);
}

#include "Quantum/Memory/MemoryUtils.inl"
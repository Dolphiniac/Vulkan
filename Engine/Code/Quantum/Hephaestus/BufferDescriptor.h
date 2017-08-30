#pragma once

#include "Quantum/Hephaestus/Declarations.h"
#include "Quantum/Core/String.h"
#include "Engine/Math/Matrix44.hpp"
#include "Quantum/Math/MathCommon.h"

#include <map>

#define START_BINDING_INDEX 0


//-----------------------------------------------------------------------------------------------
extern uint32 g_bindingIndex;
extern uint32 g_numberGlobalDescriptors;


//-----------------------------------------------------------------------------------------------
class HUniform
{
public:
	HUniform(uint32 bindingIndex, uint32 bufferSize, void* cpuBuffer);

	void InitializeBufferAndMemory();

	~HUniform();

public:
	void Push() const;

public:
	static HUniform* AddUniformBuffer(uint32 bufferSize, void* cpuBuffer);
	static void GetDescriptorSetAndLayout(HephDescriptorSetLayout* pOutLayout, HephDescriptorSet* pOutSet);
	static void InitializeDescriptorSet();
	//TODO Destroy resources on deinit

private:
	void* m_cpuBufferData = nullptr;
	uint32 m_cpuBufferSize = H_INVALID;
	HephBuffer m_buffer = H_NULL_HANDLE;
	HephDeviceMemory m_memory = H_NULL_HANDLE;

private:
	static std::map<uint32, HUniform*>* s_globalUniforms;
	static bool s_hasBeenInitialized;
	static HephDescriptorSet s_descriptorSet;
	static HephDescriptorSetLayout s_descriptorSetLayout;
	static HephDescriptorPool s_descriptorPool;
};


//-----------------------------------------------------------------------------------------------
class HGlobalShaderIncludeBuilder
{
public: 
	HGlobalShaderIncludeBuilder(const char* name, bool isStartOfStruct);
	HGlobalShaderIncludeBuilder(const char* typeName, const char* name);

public:
	static class QuString* s_globalShader;
};


//-----------------------------------------------------------------------------------------------
//When creating a new uniform buffer, first call the H_BEGIN macro, followed by H_DECLARE_MEMBER for each member
//of the struct, and lastly H_END to finalize.  Then copy all of the macros, paste in the cpp and convert _H_ to _CPP_
#define H_UNIFORM_DESCRIPTOR_H_BEGIN(structName) struct HBuffer##structName { private: static HGlobalShaderIncludeBuilder s_beginBuilder##structName;
#define H_UNIFORM_DESCRIPTOR_H_DECLARE_MEMBER(structName, varType, name) public: varType name; private: static HGlobalShaderIncludeBuilder s_memberBuilder##name;
#define H_UNIFORM_DESCRIPTOR_H_END(structName) private: static HGlobalShaderIncludeBuilder s_endBuilder##structName; public: static class HUniform* s_uniform; \
void Push() const { s_uniform->Push(); } }; extern HBuffer##structName structName;

#define H_UNIFORM_DESCRIPTOR_CPP_BEGIN(structName) HGlobalShaderIncludeBuilder HBuffer##structName::s_beginBuilder##structName = HGlobalShaderIncludeBuilder(#structName, true);
#define H_UNIFORM_DESCRIPTOR_CPP_DECLARE_MEMBER(structName, varType, name) HGlobalShaderIncludeBuilder HBuffer##structName::s_memberBuilder##name = HGlobalShaderIncludeBuilder(#varType, #name);
#define H_UNIFORM_DESCRIPTOR_CPP_END(structName) HGlobalShaderIncludeBuilder HBuffer##structName::s_endBuilder##structName = HGlobalShaderIncludeBuilder(#structName, false); \
HBuffer##structName structName; \
HUniform* HBuffer##structName::s_uniform = HUniform::AddUniformBuffer(sizeof(structName), &structName);


//-----------------------------------------------------------------------------------------------
//START UNIFORM BUFFER DECLARATIONS
//-----------------------------------------------------------------------------------------------


H_UNIFORM_DESCRIPTOR_H_BEGIN(Time)
H_UNIFORM_DESCRIPTOR_H_DECLARE_MEMBER(Time, float, time)
H_UNIFORM_DESCRIPTOR_H_END(Time)

H_UNIFORM_DESCRIPTOR_H_BEGIN(GlobalMatrices)
H_UNIFORM_DESCRIPTOR_H_DECLARE_MEMBER(GlobalMatrices, Matrix44, View)
H_UNIFORM_DESCRIPTOR_H_DECLARE_MEMBER(GlobalMatrices, Matrix44, InvView)
H_UNIFORM_DESCRIPTOR_H_DECLARE_MEMBER(GlobalMatrices, Matrix44, Projection)
H_UNIFORM_DESCRIPTOR_H_DECLARE_MEMBER(GlobalMatrices, Matrix44, InvProjection)
H_UNIFORM_DESCRIPTOR_H_END(GlobalMatrices)


//-----------------------------------------------------------------------------------------------
//END UNIFORM BUFFER DECLARATIONS
//-----------------------------------------------------------------------------------------------
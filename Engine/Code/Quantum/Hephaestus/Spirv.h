#pragma once

#include "Quantum/Core/String.h"
#include "Quantum/Hephaestus/Declarations.h"
#include "Quantum/Hephaestus/PipelineGenerator.h"


//-----------------------------------------------------------------------------------------------
namespace HSpirv
{
	enum ESpirvOpCode
	{
		OpName = 5,
		OpMemberName = 6,
		OpVariable = 59,
		OpFunction = 54,
		OpTypePointer = 32,
		OpTypeStruct = 30,
		OpTypeSampler = 26,
		OpTypeImage = 25,
		OpTypeFloat = 22,
		OpTypeInt = 21,
		OpConstant = 43,
		OpTypeSampledImage = 27,
		OpDecorate = 71,
		OpEntryPoint = 15,
		OpReturn = 253,
		OpReturnValue = 254,
		OpAccessChain = 65,
		OpLoad = 61,
		OpFNegate = 127,
		OpStore = 62,
		OpInvalid = 0
	};

	enum EDecoration
	{
		Decoration_Block = 2,
		Decoration_BufferBlock = 3,
		Decoration_Binding = 33,
		Decoration_DescriptorSet = 34,
		Decoration_Invalid = 0
	};

	enum EDimension
	{
		Dimension_2D = 1,
		Dimension_Buffer = 5,
		Dimension_SubpassData = 6,
		Dimension_Invalid = 0
	};

	struct Decoration
	{
		EDecoration type = Decoration_Invalid;
		uint32 optionalDatum = H_INVALID;
		uint32 indexOfDatum = H_INVALID;
	};

	struct PositionData
	{
		uint32 perVertexID = H_INVALID;
		uint32 positionID = H_INVALID;
		uint32 endOfMain = H_INVALID;
		const uint32 yIndex = 1;
		const uint32 glPerVertexName = QuString("gl_PerVertex");
		const uint32 glPositionName = QuString("gl_Position");

	};

	enum EExecutionModel
	{
		Execution_Vertex = 0,
		Execution_TessellationControl = 1,
		Execution_TessellationEvaluation = 2,
		Execution_Geometry = 3,
		Execution_Fragment = 4,
		Execution_Compute = 5,
		Execution_Invalid = 6
	};

	enum EStorageClass
	{
		StorageClass_UniformConstant = 0,
		StorageClass_Uniform = 2,
		StorageClass_Output = 3,
		StorageClass_Invalid = 1
	};
	struct HTemporaryReflectionData
	{
		struct ShaderVariable
		{
			uint32 name = H_INVALID;
			uint32 pointerID = H_INVALID;
			ESpirvOpCode code = OpInvalid;
			EStorageClass storageClass = StorageClass_Invalid;
			std::vector<Decoration> decorations;

			EDimension dim = Dimension_Invalid;


			void SetName(const QuString& nameStr)
			{
				name = nameStr;
			}
		};

		EExecutionModel stage;
		std::vector<ShaderVariable> vars;
		PositionData data;
		uint32 maxIDsIndex = H_INVALID;
		uint32 currentFunction = H_INVALID;
		uint32 floatID = H_INVALID;
		uint32 variableInsertionIndex = H_INVALID;
	};

	void ReflectUniforms(std::vector<uint32>& code, HReflectionData& workingReflection);
}
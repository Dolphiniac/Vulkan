#include "Quantum/Hephaestus/Spirv.h"
#include "Quantum/Hephaestus/PipelineGenerator.h"

#include <vector>


//-----------------------------------------------------------------------------------------------
const uint32 LOCAL_DESCRIPTOR_SET = 1;


//-----------------------------------------------------------------------------------------------
static uint32 GetIDCountAndAdvanceThroughHeader(const uint32* code, uint32* pOutInstructionStart)
{
	uint32 result = code[3];	//ID max count is located in word 3 of the header.  We're ignoring everything else
	*pOutInstructionStart = 5;	//Instructions start after the 5-word header

	return result;
}


//-----------------------------------------------------------------------------------------------
static void RegisterName(const uint32* code, uint32 startIndex, uint32 wordCount, HSpirv::HTemporaryReflectionData& workingData)
{
	//This operation assigns a UTF-8 name to a variable, so we'll save that in our reflection data
	uint32 id = code[startIndex];
	std::vector<char> name;
	for (uint32 wordIndex = 1; wordIndex < wordCount; wordIndex++)
	{
		union
		{
			uint32 word;
			char chars[4];
		};
		word = code[startIndex + wordIndex];

		for (uint32 charIndex = 0; charIndex < 4; charIndex++)
		{
			char thisCharacter = chars[charIndex];
			name.push_back(thisCharacter);
			if (thisCharacter == '\0')
			{
				break;
			}
		}
	}

	if (name.size() > 1)
	{
		//The name is only useful if it is not "" (size of 1 for NUL)
		QuString nameStr = &name[0];

		if (nameStr.GetHash() == workingData.data.glPerVertexName)
		{
			workingData.data.perVertexID = id;
		}
		if (nameStr == "gVertexPositions")
		{
			DebuggerPrintf("Wahoo");
		}
		workingData.vars[id].SetName(nameStr);
	}
}


//-----------------------------------------------------------------------------------------------
static void CheckMemberName(const uint32* code, uint32 startIndex, uint32 wordCount, HSpirv::HTemporaryReflectionData& workingData)
{
	uint32 id = code[startIndex];
	if (id != workingData.data.perVertexID)
	{
		return;
	}
	uint32 memberID = code[startIndex + 1];
	std::vector<char> name;
	for (uint32 workingIndex = 2; workingIndex < wordCount; workingIndex++)
	{
		union 
		{
			uint32 word;
			char chars[4];
		};
		word = code[workingIndex + startIndex];
		for (uint32 i = 0; i < 4; i++)
		{
			char c = chars[i];
			name.push_back(c);
			if (c == '\0')
			{
				break;
			}
		}
	}

	QuString nameHash = &name[0];
	if (nameHash.GetHash() == workingData.data.glPositionName)
	{
		workingData.data.positionID = memberID;
	}
}


//-----------------------------------------------------------------------------------------------
static void RegisterVariable(const uint32* code, uint32 startIndex, HSpirv::HTemporaryReflectionData& workingData)
{
	//This operation always describes the variable for which uniforms are decorated with descriptor sets and binding points,
	//and it also happens to point to some useful data for finding the descriptor type, so we'll cache it
	uint32 workingIndex = startIndex;
	uint32 typeID = code[workingIndex++];
	uint32 id = code[workingIndex++];
	uint32 storageClass = code[workingIndex];

	switch (storageClass)
	{
	case HSpirv::StorageClass_Uniform:
	case HSpirv::StorageClass_UniformConstant:
		//These two storage classes encompass ALL uniform descriptor types, so we'll only care if the specified class is one of them
		workingData.vars[id].storageClass = (HSpirv::EStorageClass)storageClass;
		workingData.vars[id].code = HSpirv::OpVariable;
		workingData.vars[id].pointerID = typeID;
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
static void RegisterPointer(const uint32* code, uint32 startIndex, HSpirv::HTemporaryReflectionData& workingData)
{
	//This is a go-between from the OpVariable that describes the uniform and the underlying type, so we need the data it points to
	uint32 workingIndex = startIndex;
	uint32 id = code[workingIndex++];
	uint32 storageClass = code[workingIndex++];
	uint32 pointedID = code[workingIndex];

	switch (storageClass)
	{
	case HSpirv::StorageClass_Uniform:
	case HSpirv::StorageClass_UniformConstant:
		//Just like OpVariable, these storage classes are used exclusively and inclusively for all uniform types
		workingData.vars[id].code = HSpirv::OpTypePointer;
		workingData.vars[id].pointerID = pointedID;
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
static void RegisterUnderlyingType(const uint32* code, uint32 startIndex, HSpirv::HTemporaryReflectionData& workingData, uint16 opID)
{
	//The information we need from OpTypeStruct, OpTypeImage, etc. is just the type it is as well as where to store it
	uint32 id = code[startIndex];
	workingData.vars[id].code = (HSpirv::ESpirvOpCode)opID;

	if (opID == HSpirv::OpTypeImage)
	{
		workingData.vars[id].dim = (HSpirv::EDimension)code[startIndex + 2];	//Differentiate between texture2D and subpassInput
	}
	else if (opID == HSpirv::OpTypeSampledImage)
	{
		workingData.vars[id].pointerID = code[startIndex + 1];
	}
}


//-----------------------------------------------------------------------------------------------
static void RegisterDecoration(const uint32* code, uint32 startIndex, uint32 wordCount, HSpirv::HTemporaryReflectionData& workingData)
{
	//We need to store any decorations on the target variable, so we can determine both what decorations exist and where to modify certain ones
	uint32 workingIndex = startIndex;
	uint32 id = code[workingIndex++];
	uint32 decorationType = code[workingIndex++];
	HSpirv::Decoration decoration;
	decoration.type = (HSpirv::EDecoration)decorationType;
	if (wordCount > workingIndex - startIndex)
	{
		decoration.optionalDatum = code[workingIndex];
		decoration.indexOfDatum = workingIndex;
	}
	workingData.vars[id].decorations.push_back(std::move(decoration));
}


//-----------------------------------------------------------------------------------------------
static void RegisterExecutionModel(const uint32* code, uint32 startIndex, HSpirv::HTemporaryReflectionData& workingData)
{
	workingData.stage = (HSpirv::EExecutionModel)code[startIndex];
}


//-----------------------------------------------------------------------------------------------
static void StartFunction(const uint32* code, uint32 startIndex, HSpirv::HTemporaryReflectionData& workingData)
{
	uint32 id = code[startIndex + 1];
	workingData.currentFunction = id;
	if (workingData.variableInsertionIndex == H_INVALID)
	{
		workingData.variableInsertionIndex = startIndex - 1;
	}
}


//-----------------------------------------------------------------------------------------------
static void EndFunction(const uint32* code, uint32 startIndex, HSpirv::HTemporaryReflectionData& workingData)
{
	uint32 functionName = workingData.vars[workingData.currentFunction].name;
	if (functionName == QuString("main"))
	{
		workingData.data.endOfMain = startIndex - 1;
	}

	workingData.currentFunction = H_INVALID;
}


//-----------------------------------------------------------------------------------------------
static void RegisterFloat(const uint32* code, uint32 startIndex, HSpirv::HTemporaryReflectionData& workingData)
{
	uint32 id = code[startIndex];
	uint32 bitWidth = code[startIndex + 1];
	if (bitWidth == 32)
	{
		//This is the one we want.  It's also guaranteed for a vertex shader, which is when we need it
		workingData.floatID = id;
	}

	//workingData.variableInsertionIndex = startIndex + 2;
}


//-----------------------------------------------------------------------------------------------
static void ParseInstructionAndAdvance(const uint32* code, uint32* pInOutCurrentWord, HSpirv::HTemporaryReflectionData& workingData, uint32* decInsertLocPtr)
{
	//The first word of any instruction is the word count for the instruction, and the opcode, bound to high and low-order ushorts, so we'll extract those
	uint32 instructionHeader = code[*pInOutCurrentWord];
	uint16 wordCount = instructionHeader >> 16;
	uint16 opID = instructionHeader & 0xFFFF;

	//I don't want to repeat myself going through the header word each time, so I'll advance through it here
	uint32 instructionBodyStart = *pInOutCurrentWord + 1;
	uint32 instructionBodyWordCount = wordCount - 1;
	switch (opID)
	{
	case HSpirv::OpName:
		RegisterName(code, instructionBodyStart, instructionBodyWordCount, workingData);
		break;
	case HSpirv::OpMemberName:
		CheckMemberName(code, instructionBodyStart, instructionBodyWordCount, workingData);
		break;
	case HSpirv::OpVariable:
		RegisterVariable(code, instructionBodyStart, workingData);
		break;
	case HSpirv::OpTypePointer:
		RegisterPointer(code, instructionBodyStart, workingData);
		break;
	case HSpirv::OpTypeStruct:
	case HSpirv::OpTypeSampler:
	case HSpirv::OpTypeImage:
	case HSpirv::OpTypeSampledImage:
		RegisterUnderlyingType(code, instructionBodyStart, workingData, opID);
		break;
	case HSpirv::OpDecorate:
		if (decInsertLocPtr)
		{
			//The first time we find a decoration, we know where we can insert new decorations
			*decInsertLocPtr = *pInOutCurrentWord;
		}
		RegisterDecoration(code, instructionBodyStart, instructionBodyWordCount, workingData);
		break;
	case HSpirv::OpEntryPoint:
		RegisterExecutionModel(code, instructionBodyStart, workingData);
		break;
	case HSpirv::OpFunction:
		StartFunction(code, instructionBodyStart, workingData);
		break;
	case HSpirv::OpReturn:
	case HSpirv::OpReturnValue:
		EndFunction(code, instructionBodyStart, workingData);
		break;
	case HSpirv::OpTypeFloat:
		RegisterFloat(code, instructionBodyStart, workingData);
		break;
	default:
		break;
	}

	//Now that we've parsed (or ignored) the instruction, we can advance past it
	*pInOutCurrentWord += wordCount;
}


//-----------------------------------------------------------------------------------------------
static void FormBindingDecoration(uint32 varID, std::vector<uint32>& decorationInstructions, uint32 bindingIndex)
{
	uint32 header = 0;
	header += 4;	//Header-ID-Decoration-Literal
	header <<= 16;	//Pushing the word count to the high-order bits
	header += HSpirv::OpDecorate;
	decorationInstructions.push_back(header);

	//Now, the ID
	decorationInstructions.push_back(varID);

	//The decoration type
	decorationInstructions.push_back(HSpirv::Decoration_Binding);

	//Finally, the binding index
	decorationInstructions.push_back(bindingIndex++);
}


//-----------------------------------------------------------------------------------------------
static void InvertPositionY(std::vector<uint32>& code, const HSpirv::HTemporaryReflectionData& tempReflectionData)
{
	std::vector<uint32> newInstructions;
	//So, to do this, we'll need a few things.  First, we need a floating point type
	//But before that, we need to allocate space for new variables
	//We'll increment based on the current max ID and then rebind the maximum
	uint32 currentVarID = tempReflectionData.vars.size();	//The bound number from the shader header

	std::vector<uint32> variableInstructions;

	//First, we need a pointer to an output float
	uint32 pointerHeader = 0;
	pointerHeader += 4;	//Word count
	pointerHeader <<= 16;
	pointerHeader += HSpirv::OpTypePointer;
	variableInstructions.push_back(pointerHeader);
	uint32 pointerID = currentVarID++;
	variableInstructions.push_back(pointerID);
	variableInstructions.push_back(HSpirv::StorageClass_Output);
	variableInstructions.push_back(tempReflectionData.floatID);

	//Next, we need a pointer that points to gl_PerVertex
	uint32 perVertexHeader = 4;
	perVertexHeader <<= 16;
	perVertexHeader += HSpirv::OpTypePointer;
	variableInstructions.push_back(perVertexHeader);
	uint32 perVertexID = currentVarID++;
	variableInstructions.push_back(perVertexID);
	variableInstructions.push_back(HSpirv::StorageClass_Output);
	variableInstructions.push_back(tempReflectionData.data.perVertexID);

	//Then, we need an output variable that points to the pointer
	uint32 variableHeader = 0;
	variableHeader += 4;
	variableHeader <<= 16;
	variableHeader += HSpirv::OpVariable;
	variableInstructions.push_back(variableHeader);
	variableInstructions.push_back(perVertexID);
	uint32 variableID = currentVarID++;
	variableInstructions.push_back(variableID);
	variableInstructions.push_back(HSpirv::StorageClass_Output);

	//Now, we need two constants for our access chain
	//But to declare them, we have to have an int type
	uint32 intHeader = 0;
	intHeader += 4;
	intHeader <<= 16;
	intHeader += HSpirv::OpTypeInt;
	variableInstructions.push_back(intHeader);
	uint32 intID = currentVarID++;
	variableInstructions.push_back(intID);
	variableInstructions.push_back(32);	//Bit width
	variableInstructions.push_back(1);	//Signedness (1 means signed)

	//Unsigned int, maybe the case?
	variableInstructions.push_back(intHeader);
	uint32 uintID = currentVarID++;
	variableInstructions.push_back(uintID);
	variableInstructions.push_back(32);
	variableInstructions.push_back(0);	//Unsigned

	//Now, for the constants
	//First, the offset into gl_PerVertex for gl_Position
	uint32 positionConstantHeader = 0;
	positionConstantHeader += 4;
	positionConstantHeader <<= 16;
	positionConstantHeader += HSpirv::OpConstant;
	variableInstructions.push_back(positionConstantHeader);
	variableInstructions.push_back(intID);
	uint32 positionConstantID = currentVarID++;
	variableInstructions.push_back(positionConstantID);
	variableInstructions.push_back(tempReflectionData.data.positionID);

	//Now, the offset into gl_Position for y
	uint32 yConstantHeader = 4;
	yConstantHeader <<= 16;
	yConstantHeader += HSpirv::OpConstant;
	variableInstructions.push_back(yConstantHeader);
	variableInstructions.push_back(uintID);
	uint32 yConstantID = currentVarID++;
	variableInstructions.push_back(yConstantID);
	variableInstructions.push_back(tempReflectionData.data.yIndex);

	//We have to insert this code into the existing code before we can move on
	uint32 oldSize1 = code.size();
	code.resize(oldSize1 + variableInstructions.size());
	memmove(&code[tempReflectionData.variableInsertionIndex + variableInstructions.size()], &code[tempReflectionData.variableInsertionIndex], (oldSize1 - tempReflectionData.variableInsertionIndex) * 4);
	memcpy(&code[tempReflectionData.variableInsertionIndex], &variableInstructions[0], variableInstructions.size() * 4);

	//Now, we need an access chain, so we can get the y member of gl_Position
	uint32 accessChainHeaderIndex = newInstructions.size();
	uint32 accessChainHeader = 0;	//We'll set this later
	newInstructions.push_back(accessChainHeader);
	newInstructions.push_back(pointerID);
	uint32 accessChainID = currentVarID++;
	newInstructions.push_back(accessChainID);
	newInstructions.push_back(variableID);
	newInstructions.push_back(positionConstantID);
	newInstructions.push_back(yConstantID);
	accessChainHeader += newInstructions.size() - accessChainHeaderIndex;
	accessChainHeader <<= 16;
	accessChainHeader += HSpirv::OpAccessChain;
	newInstructions[accessChainHeaderIndex] = accessChainHeader;

	//Now, we have to load the value from our access chain
	uint32 loadHeader = 0;
	loadHeader += 4;	//Word count
	loadHeader <<= 16;
	loadHeader += HSpirv::OpLoad;
	newInstructions.push_back(loadHeader);
	newInstructions.push_back(tempReflectionData.floatID);
	uint32 loadID = currentVarID++;
	newInstructions.push_back(loadID);
	newInstructions.push_back(accessChainID);

	//Then, we can negate the value
	uint32 negateHeader = 0;
	negateHeader = 4;	//Word count
	negateHeader <<= 16;
	negateHeader += HSpirv::OpFNegate;
	newInstructions.push_back(negateHeader);
	newInstructions.push_back(tempReflectionData.floatID);
	uint32 negateID = currentVarID++;
	newInstructions.push_back(negateID);
	newInstructions.push_back(loadID);

	//Annnnd, store it back
	uint32 storeHeader = 0;
	storeHeader += 3;	//Word count
	storeHeader <<= 16;
	storeHeader += HSpirv::OpStore;
	newInstructions.push_back(storeHeader);
	newInstructions.push_back(accessChainID);
	newInstructions.push_back(negateID);

	uint32 newEndOfMain = tempReflectionData.data.endOfMain + variableInstructions.size();

	//So, all of our instructions are made.  Now we can insert them at the end of main
	uint32 oldSize = code.size();
	code.resize(oldSize + newInstructions.size());
	memmove(&code[newEndOfMain + newInstructions.size()], &code[newEndOfMain], (oldSize - newEndOfMain) * 4);
	memcpy(&code[newEndOfMain], &newInstructions[0], newInstructions.size() * 4);

	//Finally, we update the max IDs to be 1 greater than our highest ID
	code[tempReflectionData.maxIDsIndex] = currentVarID;
}


//-----------------------------------------------------------------------------------------------
static void AddReflectionDataAndModifyCode(std::vector<uint32>& code, HReflectionData& workingReflection, HSpirv::HTemporaryReflectionData& tempReflectionData, uint32 decInsertLoc)
{
	//We'll use this vector to insert instructions into the underlying code
	std::vector<uint32> newDecorationInstructions;
	uint32 numVars = tempReflectionData.vars.size();
	for (uint32 varIndex = 0; varIndex < numVars; varIndex++)
	{
		//For each var, we'll start processing if it is an OpVariable type, which is the entry point for any uniform
		const HSpirv::HTemporaryReflectionData::ShaderVariable* currentVar = &tempReflectionData.vars[varIndex];
		if (currentVar->code != HSpirv::OpVariable)
		{
			continue;
		}

		uint32 numDecorations = currentVar->decorations.size();
		const HSpirv::Decoration* setDecoration = nullptr;
		bool foundBinding = false;
		for (uint32 decorationIndex = 0; decorationIndex < numDecorations; decorationIndex++)
		{
			//We'll parse the decoration here
			const HSpirv::Decoration* decoration = &currentVar->decorations[decorationIndex];
			switch (decoration->type)
			{
			case HSpirv::Decoration_Binding:
				//Finding a binding means the decoration is part of the global set, so useless to us
				foundBinding = true;
				break;
			case HSpirv::Decoration_DescriptorSet:
				//If we proceed, we'll need to know where this decoration is, so we can change it
				setDecoration = decoration;
				break;
			default:
				break;
			}
		}
		if (foundBinding)
		{
			//This is part of the global uniform set, so we'll ignore it
			ASSERT_OR_DIE(setDecoration->optionalDatum == 0, "If a binding exists, this had better be global\n");
			continue;
		}

		//If we've reached this point, we're shader local, so we need to do stuff
		//First, we'll change the descriptor set for this uniform to 1, for the shader-local set
		code[setDecoration->indexOfDatum] = LOCAL_DESCRIPTOR_SET;

		//We'll retrieve the binding index here and increment the reflection data's current one,
		//giving us a unique binding index for each uniform
		uint32 bindingIndex = workingReflection.currentBindingIndex++;
		
		//Next, we need to insert a new decoration binding the uniform to the binding index
		FormBindingDecoration(varIndex, newDecorationInstructions, bindingIndex);

		uint32 nameHash = H_INVALID;
		const HSpirv::HTemporaryReflectionData::ShaderVariable* pointerVar = nullptr;
		const HSpirv::HTemporaryReflectionData::ShaderVariable* underlyingVar = nullptr; 
		const HSpirv::HTemporaryReflectionData::ShaderVariable* definitelyUnderlyingVar = nullptr;
		EUniformDescriptorType type = H_DESCRIPTOR_TYPE_INVALID;

		switch (currentVar->storageClass)
		{
		case HSpirv::StorageClass_UniformConstant:
			//For uniform constant, the name is at the OpVariable var
			ASSERT_OR_DIE(currentVar->name != H_INVALID, "Why isn't this name set?\n");
			nameHash = currentVar->name;

			//Following the pointer link twice gets us to the underlying type of the uniform
			pointerVar = &tempReflectionData.vars[currentVar->pointerID];
			underlyingVar = &tempReflectionData.vars[pointerVar->pointerID];

			//Now, we'll figure out what type the underlying uniform was
			switch (underlyingVar->code)
			{
			case HSpirv::OpTypeImage:
				if (underlyingVar->dim == HSpirv::Dimension_2D)
				{
					type = H_DESCRIPTOR_TYPE_TEXTURE2D;
				}
				else if (underlyingVar->dim == HSpirv::Dimension_SubpassData)
				{
					type = H_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
				}
				else if (underlyingVar->dim == HSpirv::Dimension_Buffer)
				{
					type = H_DESCRIPTOR_TYPE_TEXEL_BUFFER;
				}
				else
				{
					ERROR_AND_DIE("We don't support this type yet\n");
				}
				break;
			case HSpirv::OpTypeSampler:
				type = H_DESCRIPTOR_TYPE_SAMPLER;
				break;
			case HSpirv::OpTypeSampledImage:
				definitelyUnderlyingVar = &tempReflectionData.vars[underlyingVar->pointerID];
				if (definitelyUnderlyingVar->code == HSpirv::OpTypeImage)
				{
					if (definitelyUnderlyingVar->dim == HSpirv::Dimension_Buffer)
					{
						type = H_DESCRIPTOR_TYPE_TEXEL_BUFFER;
					}
					else
					{
						type = H_DESCRIPTOR_TYPE_SAMPLER2D;
					}
				}
				else
				{
					ERROR_AND_DIE("Sampling non image\n");
				}
				break;
			default:
				ERROR_AND_DIE("We don't support this type yet\n");
				break;
			}

			break;
		case HSpirv::StorageClass_Uniform:
			//For a uniform, all of the important data is at the underlying type
			pointerVar = &tempReflectionData.vars[currentVar->pointerID];
			underlyingVar = &tempReflectionData.vars[pointerVar->pointerID];

			ASSERT_OR_DIE(underlyingVar->name != H_INVALID, "Why isn't this name set?\n");
			nameHash = underlyingVar->name;

			//For uniforms, the underlying type is decorated with Block or BufferBlock, which describes the type
			uint32 numUnderlyingDecorations = underlyingVar->decorations.size();
			for (uint32 underlyingDecorationIndex = 0; underlyingDecorationIndex < numUnderlyingDecorations; underlyingDecorationIndex++)
			{
				const HSpirv::Decoration* underlyingDecoration = &underlyingVar->decorations[underlyingDecorationIndex];
				switch (underlyingDecoration->type)
				{
				case HSpirv::Decoration_Block:
					type = H_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					break;
				case HSpirv::Decoration_BufferBlock:
					type = H_DESCRIPTOR_TYPE_SHADER_STORAGE_BUFFER;
					break;
				default:
					ERROR_AND_DIE("We shouldn't be running into a non-SSBO and non-UBO type here\n");
					break;
				}
			}
		}

		//We've built our reflection data, so we'll insert it into the structure
		//First, we need to make sure this name hasn't been used, for sanity reasons
		ASSERT_OR_DIE(workingReflection.descriptors.find(nameHash) == workingReflection.descriptors.end(), "Name already in use\n");

		//Now we can insert a new reflection datum
		HDescriptor newDescriptor;
		newDescriptor.bindingPoint = bindingIndex;
		newDescriptor.type = type;
		newDescriptor.shaderStage = H_SHADER_TYPE_VERTEX_BIT;
		switch (tempReflectionData.stage)
		{
		case HSpirv::Execution_Vertex:
			newDescriptor.shaderStage = H_SHADER_TYPE_VERTEX_BIT;
			break;
		case HSpirv::Execution_Fragment:
			newDescriptor.shaderStage = H_SHADER_TYPE_FRAGMENT_BIT;
			break;
		case HSpirv::Execution_Compute:
			newDescriptor.shaderStage = H_SHADER_TYPE_COMPUTE_BIT;
			break;
		case HSpirv::Execution_Geometry:
			newDescriptor.shaderStage = H_SHADER_TYPE_GEOMETRY_BIT;
			break;
		case HSpirv::Execution_TessellationControl:
			newDescriptor.shaderStage = H_SHADER_TYPE_TESSELLATION_CONTROL_BIT;
			break;
		case HSpirv::Execution_TessellationEvaluation:
			newDescriptor.shaderStage = H_SHADER_TYPE_TESSELLATION_EVALUATION_BIT;
			break;
		default:
			ERROR_AND_DIE("Cannot set shader stage for this descriptor\n");
			break;
		}

		workingReflection.descriptors.insert(std::make_pair(nameHash, newDescriptor));
	}

	//For our final step, we need to update the SPIR-V to accept our new decorations
	//First, we'll expand the final code to fit the new instructions
	if (!newDecorationInstructions.empty())
	{
		code.resize((code.size() + newDecorationInstructions.size()));

		//Next, we'll move any code from the first decoration to the end of the new decoration block
		memmove(&code[decInsertLoc + newDecorationInstructions.size()], &code[decInsertLoc], (code.size() - decInsertLoc) * 4);

		//Finally, we can copy information from the new decorations vector to the block we freed
		memcpy(&code[decInsertLoc], &newDecorationInstructions[0], newDecorationInstructions.size() * 4);

		if (tempReflectionData.variableInsertionIndex > decInsertLoc)
		{
			tempReflectionData.variableInsertionIndex += newDecorationInstructions.size();
		}
		if (tempReflectionData.data.endOfMain > decInsertLoc)
		{
			tempReflectionData.data.endOfMain += newDecorationInstructions.size();
		}
	}

	//One last thing.  If we're in the vertex shader, we need to perform a y-flip, so we need to insert some code into "main"
	if (tempReflectionData.stage == HSpirv::Execution_Vertex)
	{
		InvertPositionY(code, tempReflectionData);
	}
}


//-----------------------------------------------------------------------------------------------
void HSpirv::ReflectUniforms(std::vector<uint32>& code, HReflectionData& workingReflection)
{
	uint32 codeSize = code.size() * 4;

	uint32 numWords = code.size();	//Because spirv is organized into 4-byte words
	uint32 currentWord = 0;

	//The only thing we need from the header is the maximum number of variable IDs, for resizing our temp data vector
	uint32 numIds = GetIDCountAndAdvanceThroughHeader(&code[0], &currentWord);

	HTemporaryReflectionData data;
	data.maxIDsIndex = 3;	//Known quantity, as it's in the header
	data.vars.resize(numIds, HTemporaryReflectionData::ShaderVariable());

	uint32 decorationInsertLoc = H_INVALID;
	uint32* decInsertLocPtr = &decorationInsertLoc;
	while (currentWord < numWords)
	{
		if (decorationInsertLoc != H_INVALID)
		{
			//This variable must be set the first time we come across ANY decoration, so we know where to insert new ones
			decInsertLocPtr = nullptr;
		}
		ParseInstructionAndAdvance(&code[0], &currentWord, data, decInsertLocPtr);
	}

	//Once we've parsed this spirv module, we need to modify the code and add only the useful data to a persistent structure
	AddReflectionDataAndModifyCode(code, workingReflection, data, decorationInsertLoc);
}
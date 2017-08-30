// #include "Quantum/Hephaestus/Compiler.h"
// #include "Quantum/Hephaestus/Tokenizer.h"
// #include "Engine/Core/ErrorWarningAssert.hpp"
// 
// #include <map>
// 
// using namespace HDsl;
// 
// 
// //-----------------------------------------------------------------------------------------------
// static std::vector<uint32> g_header;
// static std::vector<uint32> g_capabilities;
// static std::vector<uint32> g_instructionImport;
// static std::vector<uint32> g_memoryModel;
// static std::vector<uint32> g_entryPoints;
// static std::vector<uint32> g_executionMode;
// static std::vector<uint32> g_names;
// static std::vector<uint32> g_decorations;
// static std::vector<uint32> g_typeDeclarations;
// static std::vector<uint32> g_forwardFunctionDeclarations;
// static std::vector<uint32> g_functionDefinitions;
// 
// 
// //-----------------------------------------------------------------------------------------------
// static std::map<ETypeName, uint32> g_typeMap;
// static std::map<uint32, HSymbol> g_symbolTable;
// static std::map<uint32, uint32> g_nativeTypes;
// static std::map<uint32, uint32> g_customTypes;
// 
// 
// //-----------------------------------------------------------------------------------------------
// static uint32 g_currentVarID = 0;
// static uint32 g_currentUniformBindingPointFrame = 0;
// static uint32 g_currentUniformBindingPointShader = 0;
// static uint32 g_currentUniformBindingPointMaterial = 0;
// static uint32 g_currentUniformBindingPointInstance = 0;
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void SpecializePunctuation(HToken& outToken)
// {
// 	using namespace HDsl;
// 	ASSERT_OR_DIE(outToken.str.GetLength() == 1, "Punctuation token may only be one character long\n");
// 
// 	char c = outToken.str[0];
// 
// 	switch (c)
// 	{
// 	case '(':
// 		outToken.punc = H_PUNCTUATION_EXPR_BEGIN;
// 		break;	
// 	case ')':
// 		outToken.punc = H_PUNCTUATION_EXPR_END;
// 		break;
// 	case '{':
// 		outToken.punc = H_PUNCTUATION_BLOCK_BEGIN;
// 		break;
// 	case '}':
// 		outToken.punc = H_PUNCTUATION_BLOCK_END;
// 		break;
// 	case ',':
// 		outToken.punc = H_PUNCTUATION_SEPARATOR;
// 		break;
// 	case ';':
// 		outToken.punc = H_PUNCTUATION_STATEMENT_END;
// 		break;
// 	default:
// 		ERROR_AND_DIE("Unrecognized punctuation token\n");
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void SpecializeOperator(HToken& outToken)
// {
// 	using namespace HDsl;
// 
// 	QuString str = outToken.str;
// 
// 	if (str == ".")
// 	{
// 		outToken.op = H_OPERATOR_MEMBER_ACCESS;
// 	}
// 	else if (str == "&&")
// 	{
// 		outToken.op = H_OPERATOR_LOGIC_AND;
// 	}
// 	else if (str == "||")
// 	{
// 		outToken.op = H_OPERATOR_LOGIC_OR;
// 	}
// 	else if (str == "!")
// 	{
// 		outToken.op = H_OPERATOR_LOGIC_NOT;
// 	}
// 	else if (str == "<")
// 	{
// 		outToken.op = H_OPERATOR_LOGIC_LESS;
// 	}
// 	else if (str == ">")
// 	{
// 		outToken.op = H_OPERATOR_LOGIC_GREATER;
// 	}
// 	else if (str == "==")
// 	{
// 		outToken.op = H_OPERATOR_LOGIC_EQUAL;
// 	}
// 	else if (str == "!=")
// 	{
// 		outToken.op = H_OPERATOR_LOGIC_NOT_EQUAL;
// 	}
// 	else if (str == "<=")
// 	{
// 		outToken.op = H_OPERATOR_LOGIC_LESS_OR_EQUAL;
// 	}
// 	else if (str == ">=")
// 	{
// 		outToken.op = H_OPERATOR_LOGIC_GREATER_OR_EQUAL;
// 	}
// 	else if (str == "+")
// 	{
// 		outToken.op = H_OPERATOR_MATH_PLUS;
// 	}
// 	else if (str == "-")
// 	{
// 		outToken.op = H_OPERATOR_MATH_MINUS;
// 	}
// 	else if (str == "*")
// 	{
// 		outToken.op = H_OPERATOR_MATH_TIMES;
// 	}
// 	else if (str == "/")
// 	{
// 		outToken.op = H_OPERATOR_MATH_DIVIDE;
// 	}
// 	else if (str == "<<")
// 	{
// 		outToken.op = H_OPERATOR_BIT_SHIFT_LEFT;
// 	}
// 	else if (str == ">>")
// 	{
// 		outToken.op = H_OPERATOR_BIT_SHIFT_RIGHT;
// 	}
// 	else if (str == "&")
// 	{
// 		outToken.op = H_OPERATOR_BIT_AND;
// 	}
// 	else if (str == "|")
// 	{
// 		outToken.op = H_OPERATOR_BIT_OR;
// 	}
// 	else if (str == "~")
// 	{
// 		outToken.op = H_OPERATOR_BIT_NOT;
// 	}
// 	else if (str == "=")
// 	{
// 		outToken.op = H_OPERATOR_ASSIGN;
// 	}
// 	else if (str == "+=")
// 	{
// 		outToken.op = H_OPERATOR_ASSIGN_PLUS;
// 	}
// 	else if (str == "-=")
// 	{
// 		outToken.op = H_OPERATOR_ASSIGN_MINUS;
// 	}
// 	else if (str == "*=")
// 	{
// 		outToken.op = H_OPERATOR_ASSIGN_TIMES;
// 	}
// 	else if (str == "/*")
// 	{
// 		outToken.op = H_OPERATOR_ASSIGN_DIVIDE;
// 	}
// 	else if (str == "<<=")
// 	{
// 		outToken.op = H_OPERATOR_ASSIGN_SHIFT_LEFT;
// 	}
// 	else if (str == ">>=")
// 	{
// 		outToken.op = H_OPERATOR_ASSIGN_SHIFT_RIGHT;
// 	}
// 	else if (str == "&=")
// 	{
// 		outToken.op = H_OPERATOR_ASSIGN_AND;
// 	}
// 	else if (str == "|=")
// 	{
// 		outToken.op = H_OPERATOR_ASSIGN_OR;
// 	}
// 	else if (str == "~=")
// 	{
// 		outToken.op = H_OPERATOR_ASSIGN_NOT;
// 	}
// 	else if (str == "++")
// 	{
// 		outToken.op = H_OPERATOR_INCREMENT;
// 	}
// 	else if (str == "--")
// 	{
// 		outToken.op = H_OPERATOR_DECREMENT;
// 	}
// 	else
// 	{
// 		ERROR_AND_DIE("Unrecognized operator token\n");
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void SpecializeToken(HToken& outToken)
// {
// 	if (outToken.type == H_TOKEN_TYPE_PUNCTUATION)
// 	{
// 		SpecializePunctuation(outToken);
// 	}
// 	else if (outToken.type == H_TOKEN_TYPE_OPERATOR)
// 	{
// 		SpecializeOperator(outToken);
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static HToken ConsumeToken(std::queue<HToken>& tokens)
// {
// 	HToken result = tokens.back();
// 	tokens.pop();
// 
// 	SpecializeToken(result);
// 
// 	return result;
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static uint32 ReserveVariableID()
// {
// 	return g_currentVarID++;
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void GetExtraArguments(std::vector<uint32>& arguments)
// {
// 	//In case no arguments are passed to this function
// 	UNUSED(arguments);
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void GetExtraArguments(std::vector<uint32>& arguments, uint32 arg)
// {
// 	arguments.push_back(arg);
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// template<typename... Extra>
// static void GetExtraArguments(std::vector<uint32>& arguments, uint32 firstArg, Extra... Args)
// {
// 	GetExtraArguments(arguments, firstArg);
// 	GetExtraArguments(arguments, Args...);
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static uint32 CreateInstructionHeader(EOperation operationCode, uint32 baseWordCount, uint32 extraArguments)
// {
// 	//An instruction header is a single word with the word count in the high-order 16 bits and the instruction ID in the low-order 16 bits
// 	uint32 result = baseWordCount + extraArguments;
// 
// 	result <<= 16;
// 	result += (uint32)operationCode;
// 
// 	return result;
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// template<typename... Extra>
// static void Decorate(uint32 variableID, EDecoration decoration, Extra... Args)
// {
// 	//<id>(target) -- decoration id -- literal arguments
// 	std::vector<uint32> extraArguments;
// 	GetExtraArguments(extraArguments, Args...);
// 	
// 	uint32 header = CreateInstructionHeader(H_OP_DECORATE, 3, extraArguments.size());
// 
// 	g_decorations.push_back(header);
// 	g_decorations.push_back(variableID);
// 	g_decorations.push_back((uint32)decoration);
// 	g_decorations.insert(g_decorations.end(), extraArguments.begin(), extraArguments.end());
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void AssignSetNumber(uint32 variableID, EScopeDesignator scopeDesignator)
// {
// 	Decorate(variableID, H_DECORATION_DESCRIPTOR_SET, (uint32)scopeDesignator);
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void AssignBindingPoint(uint32 variableID, uint32 bindingPoint)
// {
// 	Decorate(variableID, H_DECORATION_BINDING, (uint32)bindingPoint);
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void AssignScopeDesignator(uint32 variableID, EScopeDesignator scopeDesignator)
// {
// 	uint32 bindingPoint;
// 	switch (scopeDesignator)
// 	{
// 	case H_SCOPE_DESIGNATOR_FRAME:
// 		bindingPoint =  g_currentUniformBindingPointFrame++;
// 	case H_SCOPE_DESIGNATOR_SHADER:
// 		bindingPoint = g_currentUniformBindingPointShader++;
// 	case H_SCOPE_DESIGNATOR_MATERIAL:
// 		bindingPoint = g_currentUniformBindingPointMaterial++;
// 	case H_SCOPE_DESIGNATOR_INSTANCE:
// 		bindingPoint = g_currentUniformBindingPointInstance++;
// 	default:
// 		ERROR_AND_DIE("Unsupported scope designator value\n");
// 	}
// 
// 	AssignSetNumber(variableID, scopeDesignator);
// 	AssignBindingPoint(variableID, bindingPoint);
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static ETypeName ConsumeTypeName(std::queue<HToken>& tokens)
// {
// 	HToken tok = ConsumeToken(tokens);
// 
// 	ASSERT_OR_DIE(tok.type == H_TOKEN_TYPE_WORD && tok.wordType == H_WORD_TYPE_TYPENAME, "Expected type name\n");
// 
// 	return tok.typeName;
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static bool CheckIsStruct(ETypeName typeName)
// {
// 	return (typeName >= H_TYPE_NAME_BLOCK_START) && (typeName <= H_TYPE_NAME_BLOCK_END);
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void DefineSymbol(uint32 variableID, ETypeName typeName, uint32 nameHash)
// {
// 	ASSERT_OR_DIE(g_symbolTable.find(nameHash) == g_symbolTable.end(), "Redefinition of defined variable\n");
// 
// 	HSymbol symbol;
// 	symbol.id = variableID;
// 	symbol.typeName = typeName;
// 
// 	g_symbolTable.insert(std::make_pair(nameHash, symbol));
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static QuString ConsumeVariableName(std::queue<HToken>& tokens)
// {
// 	HToken varName = ConsumeToken(tokens);
// 
// 	ASSERT_OR_DIE(varName.type == H_TOKEN_TYPE_WORD && varName.wordType == H_WORD_TYPE_VARIABLE, "Expected an identifier\n");
// 
// 	return varName.str;
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void ConsumeBlockBegin(std::queue<HToken>& tokens)
// {
// 	HToken tok = ConsumeToken(tokens);
// 
// 	ASSERT_OR_DIE(tok.type == H_TOKEN_TYPE_PUNCTUATION && tok.punc == H_PUNCTUATION_BLOCK_BEGIN, "Expected a {\n");
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static bool CheckBlockEnd(const HToken& tok)
// {
// 	return tok.type == H_TOKEN_TYPE_PUNCTUATION && tok.punc == H_PUNCTUATION_BLOCK_END;
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static HTypeInfo AggregateTypeInfo(std::queue<HToken>& tokens, ETypeName typeName, uint32 nameHash)
// {
// 	HTypeInfo result;
// 
// 	result.key = nameHash;
// 	result.type = H_TYPE_INFO_CUSTOM_TYPE;
// 	result.op = H_OP_TYPE_STRUCT;
// 
// 	ConsumeBlockBegin(tokens);
// 
// 	HToken curr = ConsumeToken(tokens);
// 	while (!CheckBlockEnd(curr))
// 	{
// 		ASSERT_OR_DIE(curr.type == H_TOKEN_TYPE_WORD && curr.wordType == H_WORD_TYPE_TYPENAME, "Expected a typename\n");
// 		ETypeName typeName = curr.typeName;
// 		
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static uint32 CreateType(EOperation operation, const std::vector<HTypeInfo>& subInfos)
// {
// 	uint32 header = CreateInstructionHeader(operation, 2, subInfos.size());
// 	uint32 variableID = ReserveVariableID();
// 	g_typeDeclarations.push_back(header);
// 	g_typeDeclarations.push_back(variableID);
// 	uint32 subInfoCount = subInfos.size();
// 	for (uint32 infoIndex = 0; infoIndex < subInfoCount; infoIndex++)
// 	{
// 		const HTypeInfo& typeInfo = subInfos[infoIndex];
// 		if (typeInfo.type == H_TYPE_INFO_LITERAL)
// 		{
// 			g_typeDeclarations.push_back(typeInfo.key);
// 		}
// 		else
// 		{
// 			g_typeDeclarations.push_back(CreateOrGetType(typeInfo));
// 		}
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static uint32 CreateOrGetType(const HTypeInfo& typeInfo)
// {
// 	switch (typeInfo.type)
// 	{
// 	case H_TYPE_INFO_NATIVE_TYPE:
// 	{
// 		auto iter = g_nativeTypes.find(typeInfo.key);
// 		if (iter != g_nativeTypes.end())
// 		{
// 			return iter->second;
// 		}
// 		else
// 		{
// 			uint32 typeID = CreateType(typeInfo.op, typeInfo.extra);
// 			g_nativeTypes.insert(std::make_pair(typeInfo.key, typeID));
// 			return typeID;
// 		}
// 		break;
// 	}
// 	case H_TYPE_INFO_CUSTOM_TYPE:
// 	{
// 		auto iter = g_customTypes.find(typeInfo.key);
// 		if (iter != g_customTypes.end())
// 		{
// 			return iter->second;
// 		}
// 		else
// 		{
// 			uint32 typeID = CreateType(typeInfo.op, typeInfo.extra);
// 			g_customTypes.insert(std::make_pair(typeInfo.key, typeID));
// 			return typeID;
// 		}
// 		break;
// 	}
// 	case H_TYPE_INFO_LITERAL:
// 		return typeInfo.key;
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void CreateVariable(std::queue<HToken>& tokens, uint32 variableID, ETypeName typeName, uint32 nameHash, EStorageClass storageClass)
// {
// 	HTypeInfo typeInfo;
// 	bool isStruct = CheckIsStruct(typeName);
// 	if (isStruct)
// 	{
// 		typeInfo = AggregateTypeInfo(tokens, typeName, nameHash);
// 	}
// 	else
// 	{
// 		typeInfo = GetTypeInfoFromName(typeName);
// 	}
// 
// 	uint32 underlyingType = CreateOrGetType(typeInfo);
// 	if (storageClass == H_STORAGE_CLASS_UNIFORM_UNKNOWN)
// 	{
// 		if (isStruct)
// 		{
// 			storageClass = H_STORAGE_CLASS_UNIFORM_BLOCK;
// 			if (typeName == H_TYPE_NAME_UNIFORM_BUFFER)
// 			{
// 				Decorate(underlyingType, H_DECORATION_BLOCK);
// 			}
// 			else if (typeName == H_TYPE_NAME_SHADER_STORAGE_BUFFER)
// 			{
// 				Decorate(underlyingType, H_DECORATION_BUFFER_BLOCK);
// 			}
// 			else
// 			{
// 				ERROR_AND_DIE("Unrecognized type name for uniform struct\n");
// 			}
// 		}
// 		else
// 		{
// 			storageClass = H_STORAGE_CLASS_UNIFORM_CONSTANT;
// 		}
// 	}
// 	uint32 pointer = CreateOrGetPointer(underlyingType, storageClass);
// 	DefineSymbol(variableID, typeName, nameHash);
// 	DeclareVariable(variableID, storageClass, pointer);
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void ConsumeUniformVariableDeclaration(std::queue<HToken>& tokens, EScopeDesignator scopeDesignator)
// {
// 	uint32 variableID = ReserveVariableID();
// 	ETypeName typeName = ConsumeTypeName(tokens);
// 	QuString name = ConsumeVariableName(tokens);
// 
// 	CreateVariable(tokens, variableID, typeName, name, H_STORAGE_CLASS_UNIFORM_UNKNOWN);
// 
// 	AssignScopeDesignator(variableID, scopeDesignator);
// 
// 	HToken endDecl = ConsumeToken(tokens);
// 
// 	ASSERT_OR_DIE(endDecl.type == H_TOKEN_TYPE_PUNCTUATION && endDecl.punc == H_PUNCTUATION_STATEMENT_END, "Expected a ;\n");
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// static void ConsumeGlobalStatement(std::queue<HToken>& tokens)
// {
// 	HToken tok = ConsumeToken(tokens);
// 
// 	ASSERT_OR_DIE(tok.type == H_TOKEN_TYPE_WORD, "Global statement must begin with a scope designator or a return type for a function\n");
// 
// 	switch (tok.wordType)
// 	{
// 	case H_WORD_TYPE_SCOPE_DESIGNATOR:
// 		ConsumeUniformVariableDeclaration(tokens, tok.scopeDesignator);
// 		break;
// 	case H_WORD_TYPE_TYPENAME:
// 		ConsumeFunction(tokens, tok);
// 		break;
// 	case H_WORD_TYPE_STRUCT_KEYWORD:
// 		ConsumeStructDefinition(tokens);
// 	default:
// 		ERROR_AND_DIE("Global statement must begin with a scope designator or a return type for a function\n");
// 	}
// }
// 
// 
// //-----------------------------------------------------------------------------------------------
// HCompilation HDsl::Compile(const char* dslCode, bool newPipeline)
// {
// 	g_currentVarID = 0;
// 	if (newPipeline)
// 	{
// 		g_currentUniformBindingPointFrame = 0;
// 		g_currentUniformBindingPointShader = 0;
// 		g_currentUniformBindingPointMaterial = 0;
// 		g_currentUniformBindingPointInstance = 0;
// 	}
// 	std::queue<HToken> tokens = Tokenize(dslCode);
// 
// 	while (!tokens.empty())
// 	{
// 		ConsumeGlobalStatement(tokens);
// 	}
// }
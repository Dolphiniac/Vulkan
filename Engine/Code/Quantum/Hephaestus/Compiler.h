// #pragma once
// 
// #include "Quantum/Hephaestus/Declarations.h"
// 
// #include <vector>
// 
// 
// //-----------------------------------------------------------------------------------------------
// namespace HDsl
// {
// 	//-----------------------------------------------------------------------------------------------
// 	//ENUMERATIONS
// 	//-----------------------------------------------------------------------------------------------
// 	enum EPunctuation
// 	{
// 		H_PUNCTUATION_EXPR_BEGIN,
// 		H_PUNCTUATION_EXPR_END,
// 		H_PUNCTUATION_BLOCK_BEGIN,
// 		H_PUNCTUATION_BLOCK_END,
// 		H_PUNCTUATION_SEPARATOR,
// 		H_PUNCTUATION_STATEMENT_END
// 	};
// 
// 	//-----------------------------------------------------------------------------------------------
// 	enum EOperator
// 	{
// 		H_OPERATOR_MEMBER_ACCESS,
// 		H_OPERATOR_LOGIC_AND,
// 		H_OPERATOR_LOGIC_OR,
// 		H_OPERATOR_LOGIC_NOT,
// 		H_OPERATOR_LOGIC_LESS,
// 		H_OPERATOR_LOGIC_GREATER,
// 		H_OPERATOR_LOGIC_EQUAL,
// 		H_OPERATOR_LOGIC_NOT_EQUAL,
// 		H_OPERATOR_LOGIC_LESS_OR_EQUAL,
// 		H_OPERATOR_LOGIC_GREATER_OR_EQUAL,
// 		H_OPERATOR_MATH_PLUS,
// 		H_OPERATOR_MATH_MINUS,
// 		H_OPERATOR_MATH_TIMES,
// 		H_OPERATOR_MATH_DIVIDE,
// 		H_OPERATOR_BIT_SHIFT_LEFT,
// 		H_OPERATOR_BIT_SHIFT_RIGHT,
// 		H_OPERATOR_BIT_AND,
// 		H_OPERATOR_BIT_OR,
// 		H_OPERATOR_BIT_NOT,
// 		H_OPERATOR_ASSIGN,
// 		H_OPERATOR_ASSIGN_PLUS,
// 		H_OPERATOR_ASSIGN_MINUS,
// 		H_OPERATOR_ASSIGN_TIMES,
// 		H_OPERATOR_ASSIGN_DIVIDE,
// 		H_OPERATOR_ASSIGN_SHIFT_LEFT,
// 		H_OPERATOR_ASSIGN_SHIFT_RIGHT,
// 		H_OPERATOR_ASSIGN_AND,
// 		H_OPERATOR_ASSIGN_OR,
// 		H_OPERATOR_ASSIGN_NOT,
// 		H_OPERATOR_INCREMENT,
// 		H_OPERATOR_DECREMENT
// 	};
// 
// 
// 	//-----------------------------------------------------------------------------------------------
// 	enum EWordType
// 	{
// 		H_WORD_TYPE_SCOPE_DESIGNATOR,
// 		H_WORD_TYPE_TYPENAME,
// 		H_WORD_TYPE_VARIABLE
// 	};
// 
// 
// 	//-----------------------------------------------------------------------------------------------
// 	enum EScopeDesignator
// 	{
// 		H_SCOPE_DESIGNATOR_FRAME,
// 		H_SCOPE_DESIGNATOR_SHADER,
// 		H_SCOPE_DESIGNATOR_MATERIAL,
// 		H_SCOPE_DESIGNATOR_INSTANCE
// 	};
// 
// 
// 	//-----------------------------------------------------------------------------------------------
// 	enum ETypeName
// 	{
// 		H_TYPE_NAME_VOID,
// 		H_TYPE_NAME_IMAGE_SAMPLER,
// 		H_TYPE_NAME_UNIFORM_BUFFER,
// 		H_TYPE_NAME_SHADER_STORAGE_BUFFER,
// 		H_TYPE_NAME_BLOCK_START = H_TYPE_NAME_UNIFORM_BUFFER,
// 		H_TYPE_NAME_BLOCK_END = H_TYPE_NAME_SHADER_STORAGE_BUFFER
// 	};
// 
// 
// 	//-----------------------------------------------------------------------------------------------
// 	enum EDecoration
// 	{
// 		H_DECORATION_BLOCK = 2,
// 		H_DECORATION_BUFFER_BLOCK = 3,
// 		H_DECORATION_BINDING = 33,
// 		H_DECORATION_DESCRIPTOR_SET = 34
// 	};
// 
// 
// 	//-----------------------------------------------------------------------------------------------
// 	enum EOperation
// 	{
// 		H_OP_TYPE_STRUCT = 30,
// 		H_OP_DECORATE = 71
// 	};
// 
// 
// 	//-----------------------------------------------------------------------------------------------
// 	enum EStorageClass
// 	{
// 		H_STORAGE_CLASS_UNIFORM_CONSTANT = 0,
// 		H_STORAGE_CLASS_INPUT = 1,
// 		H_STORAGE_CLASS_UNIFORM_UNKNOWN = H_INVALID,	//Requires disambiguation
// 		H_STORAGE_CLASS_UNIFORM_BLOCK = 2,
// 		H_STORAGE_CLASS_OUTPUT = 3
// 	};
// 
// 
// 	//-----------------------------------------------------------------------------------------------
// 	enum ETypeInfoType
// 	{
// 		H_TYPE_INFO_SCALAR,
// 		H_TYPE_INFO_VECTOR,
// 		H_TYPE_INFO_MATRIX,
// 		H_TYPE_INFO_STRUCT
// 	};
// 	//-----------------------------------------------------------------------------------------------
// 	//END ENUMERATIONS
// 	//-----------------------------------------------------------------------------------------------
// 
// 	//-----------------------------------------------------------------------------------------------
// 	//DATA TYPES
// 	//-----------------------------------------------------------------------------------------------
// 	struct HCompilation
// 	{
// 		std::vector<uint32> code;
// 		//Reflection data should be included here
// 	};
// 
// 
// 	//-----------------------------------------------------------------------------------------------
// 	struct HSymbol
// 	{
// 		uint32 id;
// 		ETypeName typeName;
// 	};
// 
// 
// 	//-----------------------------------------------------------------------------------------------
// 	struct HTypeInfo
// 	{
// 		ETypeInfoType type;
// 		uint32 key;
// 		uint32 id;
// 		bool isMember;
// 		uint32 arrElems;
// 		std::vector<HTypeInfo> members;
// 	};
// 	//-----------------------------------------------------------------------------------------------
// 	//END DATA TYPES
// 	//-----------------------------------------------------------------------------------------------
// 
// 	//-----------------------------------------------------------------------------------------------
// 	//FUNCTIONS
// 	//-----------------------------------------------------------------------------------------------
// 	HCompilation Compile(const char* dslCode, bool newPipeline);
// 	//-----------------------------------------------------------------------------------------------
// 	//END FUNCTIONS
// 	//-----------------------------------------------------------------------------------------------
// }
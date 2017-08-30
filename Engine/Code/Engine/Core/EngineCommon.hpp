#pragma once


//-----------------------------------------------------------------------------------------------
#define __SHADERS__
#define MILLION * 1000000
#define BILLION MILLION * 1000
#define KB * 1024
#define MB KB * 1024
#define GB MB * 1024

//Only use for fixed arrays
//#define ARRAY_LENGTH(ArrayObj) (sizeof(ArrayObj)/sizeof(*ArrayObj))
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#ifdef __cplusplus
template<typename T>
struct ExplicitTemplateAlias
{
	using type = T;
};

//Enclose templated typename in this alias (Non-Deducible<Type>) to prevent automatic deduction
template<typename T>
using ND = typename ExplicitTemplateAlias<T>::type;
#endif
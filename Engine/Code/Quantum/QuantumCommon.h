#pragma once

//This header contains global defines, typedefs, and ease-of-use macros for the entire Quantum engine
typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;
typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

typedef uint8 byte;


//-----------------------------------------------------------------------------------------------
#define STATIC
#define GLOBAL
#define UNUSED(...) __VA_ARGS__
#define ARRAY_LENGTH(x) sizeof(x) / sizeof(x[0])
#define SAFE_DELETE(x) if (x) delete x; x = nullptr
#define SAFE_DELETE_ARRAY(x) if (x) delete[] x; x = nullptr
#define SAFE_DELETE_PTR_ARRAY(x, count) for (uint32 i = 0; i < count; i++) { SAFE_DELETE(x[i]); } SAFE_DELETE_ARRAY(x)
#define INVALID_UINT ~0U

#define here(msgText) static_assert(false, msgText)

#define FOR_COUNT(idxName, count) for (uint32 idxName = 0; idxName < count; idxName++)
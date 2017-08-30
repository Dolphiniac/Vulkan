#include "Quantum/Core/String.h"
#include "Quantum/Renderer/Color.h"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>


//-----------------------------------------------------------------------------------------------
QuString::QuString()
	: m_capacity(s_MIN_STRING_MEMORY_BUFFER)
	, m_length(0)
{
	m_stringPtr = m_internalBuffer;
	*m_stringPtr = '\0';
}


//-----------------------------------------------------------------------------------------------
QuString::QuString(const char* otherString)
{
	m_length = strlen(otherString);
	m_capacity = m_length + 1;
	if (m_capacity > s_MIN_STRING_MEMORY_BUFFER)
	{
		m_stringPtr = new char[m_capacity];
	}
	else
	{
		m_stringPtr = m_internalBuffer;
	}
	if (m_length >= 0)
	{
		strcpy_s(m_stringPtr, m_capacity, otherString);
	}
}


//-----------------------------------------------------------------------------------------------
QuString::QuString(const QuString& otherString)
{
	m_length = otherString.GetLength();
	m_capacity = m_length + 1;
	if (m_capacity > s_MIN_STRING_MEMORY_BUFFER)
	{
		m_stringPtr = new char[m_capacity];
	}
	else
	{
		m_stringPtr = m_internalBuffer;
	}

	if (m_length > 0)
	{
		strcpy_s(m_stringPtr, m_capacity, otherString.GetRaw());
	}
}


//-----------------------------------------------------------------------------------------------
QuString::QuString(QuString&& otherString)
{
	m_length = otherString.m_length;
	m_capacity = otherString.m_capacity;

	if (otherString.m_stringPtr == otherString.m_internalBuffer)
	{
		m_stringPtr = m_internalBuffer;
		strcpy_s(m_stringPtr, otherString.m_length + 1, otherString.m_stringPtr);
	}
	else
	{
		m_stringPtr = otherString.m_stringPtr;
	}

	otherString.m_stringPtr = nullptr;
}


//-----------------------------------------------------------------------------------------------
QuString::QuString(uint32 inputInt)
{
	m_stringPtr = m_internalBuffer;
	*m_stringPtr = '\0';
	m_capacity = s_MIN_STRING_MEMORY_BUFFER;

	char intString[s_MIN_STRING_MEMORY_BUFFER - 1]; //Ensuring we're within capacity
	sprintf_s(intString, "%u", inputInt);

	m_length = strlen(intString);

	strcpy_s(m_stringPtr, m_length + 1, intString);
}


//-----------------------------------------------------------------------------------------------
QuString::~QuString()
{
	if (m_stringPtr != m_internalBuffer)
	{
		SAFE_DELETE_ARRAY(m_stringPtr);
	}
}


//-----------------------------------------------------------------------------------------------
void QuString::Push(char c)
{
	char buffer[2];
	buffer[0] = c;
	buffer[1] = '\0';

	*this += buffer;
}


//-----------------------------------------------------------------------------------------------
void QuString::operator=(const QuString& otherString)
{
	m_length = otherString.GetLength();
	m_capacity = m_length + 1;
	if (m_capacity > s_MIN_STRING_MEMORY_BUFFER)
	{
		m_stringPtr = new char[m_capacity];
	}
	else
	{
		m_stringPtr = m_internalBuffer;
	}

	if (m_length > 0)
	{
		strcpy_s(m_stringPtr, m_capacity, otherString.GetRaw());
	}
}


//-----------------------------------------------------------------------------------------------
QuString QuString::operator+(const QuString& otherString) const
{
	QuString result = *this;
	size_t otherLen = otherString.m_length;

	bool changedSize = false;
	while ((result.m_length + otherLen + 1) > result.m_capacity)
	{
		result.m_capacity *= 2;
		changedSize = true;
	}

	if (changedSize && result.m_capacity > s_MIN_STRING_MEMORY_BUFFER)
	{
		char* newBuff = new char[result.m_capacity];
		memcpy(newBuff, result.m_stringPtr, result.m_length);
		if (result.m_stringPtr != result.m_internalBuffer)
		{
			delete[] result.m_stringPtr;
		}
		result.m_stringPtr = newBuff;
	}


	strcpy_s(result.m_stringPtr + result.m_length, otherString.m_length + 1, otherString.m_stringPtr);
	result.m_length += otherLen;

	return result;
}


//-----------------------------------------------------------------------------------------------
QuString QuString::operator+(const char* otherString) const
{
	return *this + QuString(otherString);
}


//-----------------------------------------------------------------------------------------------
void QuString::operator+=(const QuString& otherString)
{
	size_t otherLen = otherString.m_length;

	bool changedSize = false;
	while ((m_length + otherLen + 1) > m_capacity)
	{
		m_capacity *= 2;
		changedSize = true;
	}

	if (changedSize)
	{
		char* newBuff = new char[m_capacity];
		memcpy(newBuff, m_stringPtr, m_length);
		if (m_stringPtr != m_internalBuffer)
		{
			delete[] m_stringPtr;
		}
		m_stringPtr = newBuff;
	}

	strcpy_s(m_stringPtr + m_length, otherString.m_length + 1, otherString.m_stringPtr);
	m_length += otherString.m_length;
}


//-----------------------------------------------------------------------------------------------
// QuString& QuString::operator=(QuString&& otherString)
// {
// 	if (this != &otherString)
// 	{
// 		if (m_stringPtr != m_internalBuffer)
// 		{
// 			delete[] m_stringPtr;
// 		}
// 
// 		m_stringPtr = otherString.m_stringPtr;
// 		m_length = otherString.m_length;
// 		m_capacity = otherString.m_capacity;
// 
// 		otherString.m_stringPtr = nullptr;
// 	}
// 
// 	return *this;
// }


//-----------------------------------------------------------------------------------------------
bool QuString::operator==(const QuString& otherString) const
{
	return GetHash() == otherString.GetHash();
}


//-----------------------------------------------------------------------------------------------
bool QuString::operator!=(const QuString& otherString) const
{
	return !(*this == otherString);
}


//-----------------------------------------------------------------------------------------------
char& QuString::operator[](uint32 index) const
{
	ASSERT_OR_DIE(index < GetLength(), "Invalid index for char retrieval\n");

	return m_stringPtr[index];
}


//-----------------------------------------------------------------------------------------------
STATIC QuString QuString::F(const QuString& formatString, ...)
{
#ifndef __X64
	char textLiteral[2048];
	va_list vargs;
	uint32 ptrSize = sizeof(void*);

	//Getting past the annoyance of not using reference types with vargs by manually moving the ptr myself
	__asm
	{
		lea eax, [formatString];
		add eax, [ptrSize];
		mov[vargs], eax;
	}
	vsnprintf_s(textLiteral, 2048, _TRUNCATE, formatString.GetRaw(), vargs);
	textLiteral[2047] = '\0';

	return textLiteral;
#else
	return "Cannot execute necessary assembly in x64 architecture.";
#endif
}


//-----------------------------------------------------------------------------------------------
static bool ConvertHexToDecimalByte(char* startPtr, uint8& outByte)
{
	outByte = 0;
	char curr = startPtr[0];
	if (curr >= '0' && curr <= '9')
	{
		outByte += (curr - '0') * 16;
	}
	else if (curr >= 'A' && curr <= 'F')
	{
		outByte += (curr - 'A' + 10) * 16;
	}
	else
	{
		return false;
	}

	curr = startPtr[1];
	if (curr >= '0' && curr <= '9')
	{
		outByte += (curr - '0');
	}
	else if (curr >= 'A' && curr <= 'F')
	{
		outByte += (curr - 'A' + 10);
	}
	else
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
QuColor QuString::AsColor() const
{
	//Hex code value, could be 6 or 8 chars, specifying alpha
	if (m_length != 6 && m_length != 8)
	{
		//Bad hex value
		return QuColor::BAD_COLOR;
	}

	uint8 r, g, b, a;

	if (!ConvertHexToDecimalByte(m_stringPtr, r))
	{
		return QuColor::BAD_COLOR;
	}
	if (!ConvertHexToDecimalByte(m_stringPtr + 2, g))
	{
		return QuColor::BAD_COLOR;
	}
	if (!ConvertHexToDecimalByte(m_stringPtr + 4, b))
	{
		return QuColor::BAD_COLOR;
	}

	if (m_length == 8)
	{
		if (!ConvertHexToDecimalByte(m_stringPtr + 6, a))
		{
			return QuColor::BAD_COLOR;
		}
	}
	else
	{
		a = 255;
	}

	return QuColor(r, g, b, a);
}


//-----------------------------------------------------------------------------------------------
float QuString::AsFloat() const
{
	return (float)atof(m_stringPtr);
}


//-----------------------------------------------------------------------------------------------
int QuString::AsInt() const
{
	return atoi(m_stringPtr);
}


//-----------------------------------------------------------------------------------------------
QuVector2 QuString::AsVec2() const
{
	QuString workingString = *this;
	workingString.Trim();

	std::vector<QuString> components = workingString.SplitOnDelimiter(',');
	if (components.size() != 2)
	{
		return QuVector2::Zero;
	}

	if (!components[0].IsNumeric())
	{
		return QuVector2::Zero;
	}
	if (!components[1].IsNumeric())
	{
		return QuVector2::Zero;
	}

	QuVector2 result;

	result.x = components[0].AsFloat();
	result.y = components[1].AsFloat();

	return result;
}


//-----------------------------------------------------------------------------------------------
QuVector4 QuString::AsVec4() const
{
	QuString workingString = *this;
	workingString.Trim();

	std::vector<QuString> components = workingString.SplitOnDelimiter(',');
	if (components.size() != 4)
	{
		return QuVector4::Zero;
	}

	if (!components[0].IsNumeric())
	{
		return QuVector4::Zero;
	}
	if (!components[1].IsNumeric())
	{
		return QuVector4::Zero;
	}
	if (!components[2].IsNumeric())
	{
		return QuVector4::Zero;
	}
	if (!components[3].IsNumeric())
	{
		return QuVector4::Zero;
	}

	QuVector4 result;

	result.x = components[0].AsFloat();
	result.y = components[1].AsFloat();
	result.z = components[2].AsFloat();
	result.w = components[3].AsFloat();

	return result;
}


//-----------------------------------------------------------------------------------------------
QuString QuString::ToUpper() const
{
	QuString result = *this;

	for (uint32 i = 0; i < result.m_length; i++)
	{
		char& c = result.m_stringPtr[i];

		if (c >= 'a' && c <= 'z')
		{
			c -= 'a';
			c += 'A';
		}
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
int QuString::GetOccurrencesOf(char c) const
{
	int result = 0;
	for (uint32 i = 0; i < m_length; i++)
	{
		if (m_stringPtr[i] == c)
		{
			result++;
		}
	}
	
	return result;
}


//-----------------------------------------------------------------------------------------------
bool QuString::IsNumeric() const
{
	uint32 i = 0;
	if (m_stringPtr[0] == '-')
	{
		i = 1;
	}
	for (; i < m_length; i++)
	{
		char c = m_stringPtr[i];
		if (c == '.')
		{
			continue;
		}

		if (c >= '0' && c <= '9')
		{
			continue;
		}

		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
std::vector<QuString> QuString::SplitOnDelimiter(char delim) const
{
	std::vector<QuString> result;

	QuString current;

	int numCharsAccumulated = 0;
	for (uint32 i = 0; i < m_length; i++)
	{
		char c = m_stringPtr[i];
		if (c == delim)
		{
			if (numCharsAccumulated > 0)
			{
				result.push_back(current);
				current = "";
				numCharsAccumulated = 0;
			}
		}
		else
		{
			current.Push(c);
			numCharsAccumulated++;
		}
	}

	if (numCharsAccumulated > 0)
	{
		result.push_back(current);
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
void QuString::Trim()
{
	for (uint32 i = 0; i < m_length; i++)
	{
		char c = m_stringPtr[i];
		if (c == ' ' || c == '\t' || c == '\n')
		{
			memmove(m_stringPtr + i, m_stringPtr + i + 1, m_length - i - 1);
			m_length--;
			i--;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void QuString::Clear()
{
	*this = "";
}
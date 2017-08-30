#pragma once

#include "Quantum/Renderer/Color.h"
#include "Quantum/Math/MathCommon.h"

#include <xstddef>
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
typedef uint32 QuHash;


//-----------------------------------------------------------------------------------------------
class QuString
{
	static const int8 s_MIN_STRING_MEMORY_BUFFER = 32;

public:
	//-----------------------------------------------------------------------------------------------
	//CTORS AND DTORS
	QuString();
	QuString(const char* otherString);
	QuString(const QuString& otherString);
	QuString(QuString&& otherString);
	QuString(uint32 inputInt);
	~QuString();
	//-----------------------------------------------------------------------------------------------

public:
	//-----------------------------------------------------------------------------------------------
	//GETTERS AND SETTERS
	size_t GetLength() const { return m_length; }
	size_t GetCapacity() const { return m_capacity; }
	const char* GetRaw() const { return m_stringPtr; }
	QuHash GetHash() const { return std::hash<std::string>{}(GetRaw()); }
	void Push(char c);
	//-----------------------------------------------------------------------------------------------
	
public:
	//-----------------------------------------------------------------------------------------------
	//OPERATORS
	void operator=(const QuString& otherString);
	QuString operator+(const QuString& otherString) const;
	QuString operator+(const char* otherString) const;
	void operator+=(const QuString& otherString);
	//QuString& operator=(QuString&& otherString);
	bool operator==(const QuString& otherString) const;
	bool operator!=(const QuString& otherString) const;
	char& operator[](uint32 index) const;
	operator QuHash() const { return GetHash(); } //Easy hashing
	//-----------------------------------------------------------------------------------------------

public:
	//-----------------------------------------------------------------------------------------------
	//UTILITY FUNCTIONS
	static QuString F(const QuString& formatString, ...);
	QuColor AsColor() const;
	float AsFloat() const;
	int AsInt() const;
	QuVector2 AsVec2() const;
	QuVector4 AsVec4() const;
	template<typename T> T ToDatum() const;
	QuString ToUpper() const;
	int GetOccurrencesOf(char c) const;
	bool IsNumeric() const;
	std::vector<QuString> SplitOnDelimiter(char delim) const;
	void Trim();
	bool IsEmpty() const { return GetLength() == 0; }
	void Clear();
	char GetLast() const { return m_stringPtr[m_length - 1]; }
	//-----------------------------------------------------------------------------------------------

private:
	char* m_stringPtr;
	char m_internalBuffer[s_MIN_STRING_MEMORY_BUFFER];
	size_t m_capacity;
	size_t m_length;
};
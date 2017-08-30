#pragma once

#include "FileUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Renderer/Rgba.hpp"

#include <deque>


//-----------------------------------------------------------------------------------------------
class BinaryReader
{
public:
	inline BinaryReader(const std::string& filePath);
	template<typename T> inline void Read(T& outData);
	bool IsEmpty() const { return m_numEntries == 0; }
	char Front() { if (!IsEmpty()) return m_byteBuffer[0]; else return '\0'; }
	inline void PopFront(); 
private:
	char* m_byteBuffer;
	char* origPointer;
	bool hasDeletedBuffer = false;
	int m_numEntries;
};


//-----------------------------------------------------------------------------------------------
inline BinaryReader::BinaryReader(const std::string& filePath)
{
	m_byteBuffer = GetBinaryFile(filePath, m_numEntries);
	origPointer = m_byteBuffer;
}


//-----------------------------------------------------------------------------------------------
template<> inline void BinaryReader::Read(char& outData)
{
	outData = Front();//m_byteBuffer.front();
	PopFront();//m_byteBuffer.pop_front();
}


//-----------------------------------------------------------------------------------------------
template<> inline void BinaryReader::Read(int& outData)
{
	union
	{
		int iData;
		char bytes[sizeof(outData)];
	};
	for (size_t byte = 0; byte < sizeof(outData); byte++)
	{
		bytes[byte] = Front();// m_byteBuffer.front();
		PopFront();// m_byteBuffer.pop_front();
	}
	outData = iData;
}


//-----------------------------------------------------------------------------------------------
template<> inline void BinaryReader::Read(unsigned short& outData)
{
	union
	{
		unsigned short iData;
		char bytes[sizeof(outData)];
	};
	for (size_t byte = 0; byte < sizeof(outData); byte++)
	{
		bytes[byte] = Front();// m_byteBuffer.front();
		PopFront();// m_byteBuffer.pop_front();
	}
	outData = iData;
}


//-----------------------------------------------------------------------------------------------
template<> inline void BinaryReader::Read(float& outData)
{
	union
	{
		float iData;
		char bytes[sizeof(outData)];
	};
	for (size_t byte = 0; byte < sizeof(outData); byte++)
	{
		bytes[byte] = Front();// m_byteBuffer.front();
		PopFront();// m_byteBuffer.pop_front();
	}
	outData = iData;
}


//-----------------------------------------------------------------------------------------------
template<> inline void BinaryReader::Read(Vector3& outData)
{
	Read(outData.x);
	Read(outData.y);
	Read(outData.z);
}


//-----------------------------------------------------------------------------------------------
template<> inline void BinaryReader::Read(Vector4& outData)
{
	Read(outData.x);
	Read(outData.y);
	Read(outData.z);
	Read(outData.w);
}


//-----------------------------------------------------------------------------------------------
template<> inline void BinaryReader::Read(IntVector4& outData)
{
	Read(outData.x);
	Read(outData.y);
	Read(outData.z);
	Read(outData.w);
}


//-----------------------------------------------------------------------------------------------
template<> inline void BinaryReader::Read(Vector2& outData)
{
	Read(outData.x);
	Read(outData.y);
}


//-----------------------------------------------------------------------------------------------
template<> inline void BinaryReader::Read(Matrix44& outData)
{
	for (int i = 0; i < 16; i++)
	{
		Read(outData.data[i]);
	}
}


//-----------------------------------------------------------------------------------------------
template<> inline void BinaryReader::Read(Rgba& outData)
{
	outData.r = Front();// m_byteBuffer.front();
	PopFront();// m_byteBuffer.pop_front();
	outData.g = Front();// m_byteBuffer.front();
	PopFront();// m_byteBuffer.pop_front();
	outData.b = Front();// m_byteBuffer.front();
	PopFront();// m_byteBuffer.pop_front();
	outData.a = Front();// m_byteBuffer.front();
	PopFront();// m_byteBuffer.pop_front();
}


//-----------------------------------------------------------------------------------------------
template<> inline void BinaryReader::Read(std::string& outData)
{
	outData = "";
	for (;;)
	{
		char c = Front();// m_byteBuffer.front();
		PopFront();// m_byteBuffer.pop_front();
		outData.push_back(c);
		if (c == '\0')
		{
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void BinaryReader::PopFront()
{
	m_byteBuffer += sizeof(char);
	m_numEntries--;
	if (m_numEntries <= 0)
	{
		if (!hasDeletedBuffer)
		{
			free(origPointer);
			hasDeletedBuffer = true;
		}
	}
}
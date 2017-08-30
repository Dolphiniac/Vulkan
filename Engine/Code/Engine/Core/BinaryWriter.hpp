#pragma once

#include <vector>

#include "Engine/Core/FileUtils.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Renderer/Rgba.hpp"


//-----------------------------------------------------------------------------------------------
class BinaryWriter
{
public:
	inline BinaryWriter();
	template<typename T> inline void Write(const T& data);
	inline void WriteBufferToFile(const std::string& filePath);
private:
	std::vector<char> m_byteBuffer;
};


//-----------------------------------------------------------------------------------------------
inline BinaryWriter::BinaryWriter()
{
	m_byteBuffer.reserve(4096);	//Smallest num bytes in cache
}


//-----------------------------------------------------------------------------------------------
template <> inline void BinaryWriter::Write(const char& data)
{
	m_byteBuffer.push_back(data);
}


//-----------------------------------------------------------------------------------------------
template <> inline void BinaryWriter::Write(const int& data)
{
	union
	{
		int iData;
		char bytes[sizeof(data)];
	};
	iData = data;
	for (size_t byte = 0; byte < sizeof(data); byte++)
	{
		m_byteBuffer.push_back(bytes[byte]);
	}
}


//-----------------------------------------------------------------------------------------------
template <> inline void BinaryWriter::Write(const unsigned int& data)
{
	union
	{
		unsigned int iData;
		char bytes[sizeof(data)];
	};
	iData = data;
	for (size_t byte = 0; byte < sizeof(data); byte++)
	{
		m_byteBuffer.push_back(bytes[byte]);
	}
}


//-----------------------------------------------------------------------------------------------
template<> inline void BinaryWriter::Write(const float& data)
{
	union
	{
		float iData;
		char bytes[sizeof(data)];
	};
	iData = data;
	for (size_t byte = 0; byte < sizeof(data); byte++)
	{
		m_byteBuffer.push_back(bytes[byte]);
	}
}


//-----------------------------------------------------------------------------------------------
template <> inline void BinaryWriter::Write(const Vector3& data)
{
	Write(data.x);
	Write(data.y);
	Write(data.z);
}


//-----------------------------------------------------------------------------------------------
template <> inline void BinaryWriter::Write(const Vector4& data)
{
	Write(data.x);
	Write(data.y);
	Write(data.z);
	Write(data.w);
}


//-----------------------------------------------------------------------------------------------
template <> inline void BinaryWriter::Write(const IntVector4& data)
{
	Write(data.x);
	Write(data.y);
	Write(data.z);
	Write(data.w);
}


//-----------------------------------------------------------------------------------------------
template <> inline void BinaryWriter::Write(const Vector2& data)
{
	Write(data.x);
	Write(data.y);
}


//-----------------------------------------------------------------------------------------------
template <> inline void BinaryWriter::Write(const Rgba& data)
{
	m_byteBuffer.push_back(data.r);
	m_byteBuffer.push_back(data.g);
	m_byteBuffer.push_back(data.b);
	m_byteBuffer.push_back(data.a);
}


//-----------------------------------------------------------------------------------------------
template <> inline void BinaryWriter::Write(const unsigned short& data)
{
	union
	{
		unsigned short iData;
		char bytes[sizeof(data)];
	};
	iData = data;
	for (size_t byte = 0; byte < sizeof(data); byte++)
	{
		m_byteBuffer.push_back(bytes[byte]);
	}
}


//-----------------------------------------------------------------------------------------------
template<> inline void BinaryWriter::Write(const std::string& data)
{
	for (char c : data)
	{
		Write(c);
	}
	Write('\0');
}


//-----------------------------------------------------------------------------------------------
template<> inline void BinaryWriter::Write(const Matrix44& data)
{
	for (int i = 0; i < 16; i++)
	{
		Write(data.data[i]);
	}
}


//-----------------------------------------------------------------------------------------------
inline void BinaryWriter::WriteBufferToFile(const std::string& filePath)
{
	SaveBinaryFileFromBuffer(filePath, m_byteBuffer);
}
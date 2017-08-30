#include "Engine/Core/FileUtils.hpp"

#include <stdio.h>
#include <io.h>


//-----------------------------------------------------------------------------------------------
bool LoadBinaryFileToBuffer(const std::string& filePath, std::vector<unsigned char>& out_buffer)
{
	FILE* inFile;

	errno_t err = fopen_s(&inFile, filePath.c_str(), "rb");
	if (err != 0)
		return false;

	fseek(inFile, SEEK_SET, SEEK_END);
	int buffSize = ftell(inFile);
	rewind(inFile);

	out_buffer.resize(buffSize);

	fread(&out_buffer[0], 1, buffSize, inFile);
	fclose(inFile);
	return true;
}


//-----------------------------------------------------------------------------------------------
char* GetBinaryFile(const std::string& filePath, int& numEntries)
{
	FILE* inFile;

	errno_t err = fopen_s(&inFile, filePath.c_str(), "rb");
	if (err != 0)
		return false;

	fseek(inFile, SEEK_SET, SEEK_END);
	int buffSize = ftell(inFile);
	rewind(inFile);

	char* buffer = (char*)malloc(sizeof(char) * buffSize);

	fread(buffer, 1, buffSize, inFile);
	fclose(inFile);
	numEntries = buffSize;
	return buffer;
}


//-----------------------------------------------------------------------------------------------
bool LoadBinaryFileToBuffer(const std::string& filePath, std::vector<char>& out_buffer)
{
	FILE* inFile;

	errno_t err = fopen_s(&inFile, filePath.c_str(), "rb");
	if (err != 0)
		return false;

	fseek(inFile, SEEK_SET, SEEK_END);
	int buffSize = ftell(inFile);
	rewind(inFile);

	out_buffer.resize(buffSize);

	fread(&out_buffer[0], 1, buffSize, inFile);
	fclose(inFile);
	return true;
}


//-----------------------------------------------------------------------------------------------
bool LoadBinaryFileToBuffer(const std::string& filePath, std::deque<char>& out_buffer)
{
	std::vector<char> initBuffer;
	bool success = LoadBinaryFileToBuffer(filePath, initBuffer);
	for (const char& c : initBuffer)
	{
		out_buffer.push_back(c);
	}
	return success;
}


//-----------------------------------------------------------------------------------------------
bool SaveBinaryFileFromBuffer(const std::string& filePath, const std::vector<unsigned char>& buffer)
{
	FILE* outFile;

	errno_t err = fopen_s(&outFile, filePath.c_str(), "wb");
	if (err != 0)
		return false;

	fwrite(&buffer[0], 1, buffer.size(), outFile);
	fclose(outFile);
	return true;
}


//-----------------------------------------------------------------------------------------------
bool SaveBinaryFileFromBuffer(const std::string& filePath, const std::vector<char>& buffer)
{
	FILE* outFile;

	errno_t err = fopen_s(&outFile, filePath.c_str(), "wb");
	if (err != 0)
		return false;

	fwrite(&buffer[0], 1, buffer.size(), outFile);
	fclose(outFile);
	return true;
}


//-----------------------------------------------------------------------------------------------
std::vector<std::string> FindFilesWith(const std::string& searchDirectory, const std::string& toFind)
{
	std::vector<std::string> results;
	_finddata_t fileInfo;
	std::string paddedSearchDir = searchDirectory + "/";
	std::string searchPattern = paddedSearchDir + toFind;
	intptr_t searchHandle = _findfirst(searchPattern.c_str(), &fileInfo);

	int error = 0;
	while (!error && searchHandle != -1)
	{
		results.push_back(paddedSearchDir + fileInfo.name);
		error = _findnext(searchHandle, &fileInfo);
	}

	_findclose(searchHandle);

	return results;
}


//-----------------------------------------------------------------------------------------------
bool DoesFileExist(const std::string& filepath)
{
	_finddata_t fileInfo;
	intptr_t searchHandle = _findfirst(filepath.c_str(), &fileInfo);
	int result = searchHandle;
	_findclose(searchHandle);
	return result != -1;
}
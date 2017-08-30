#pragma once

#include <string>
#include <vector>
#include <deque>


//-----------------------------------------------------------------------------------------------
bool LoadBinaryFileToBuffer(const std::string& filePath, std::vector<unsigned char>& out_buffer);
char* GetBinaryFile(const std::string& filePath, int& numEntries);
bool LoadBinaryFileToBuffer(const std::string& filePath, std::vector<char>& out_buffer);
bool LoadBinaryFileToBuffer(const std::string& filePath, std::deque<char>& out_buffer);
bool SaveBinaryFileFromBuffer(const std::string& filePath, const std::vector<unsigned char>& buffer);
bool SaveBinaryFileFromBuffer(const std::string& filePath, const std::vector<char>& buffer);
std::vector<std::string> FindFilesWith(const std::string& searchDirectory, const std::string& toFind);
bool DoesFileExist(const std::string& filepath);
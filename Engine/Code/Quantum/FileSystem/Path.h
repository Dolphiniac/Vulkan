#pragma once

#include "Quantum/Core/String.h"

#include <vector>


//-----------------------------------------------------------------------------------------------
class QuPath
{
public:
	static void InitializeDataPath();

public:
	static QuPath s_dataPath;

private:
	void InitializeDirectory();

private:
	QuString m_absolutePath;
	std::vector<QuPath> m_subDirectories;
	std::vector<QuString> m_filesInDirectory;
};
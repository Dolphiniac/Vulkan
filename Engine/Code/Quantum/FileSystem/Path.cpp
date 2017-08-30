#include "Quantum/FileSystem/Path.h"
#include "Engine/Core/ErrorWarningAssert.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>


//-----------------------------------------------------------------------------------------------
STATIC QuPath QuPath::s_dataPath;


//-----------------------------------------------------------------------------------------------
STATIC void QuPath::InitializeDataPath()
{
	const int32 k_bufferLength = 1024;
	char buffer[k_bufferLength];
	GetCurrentDirectoryA(k_bufferLength, buffer);
	s_dataPath.m_absolutePath = buffer;
	s_dataPath.m_absolutePath += "\\Data\\";

	s_dataPath.InitializeDirectory();
	DebuggerPrintf("");
}


//-----------------------------------------------------------------------------------------------
void QuPath::InitializeDirectory()
{
	const char* searchPattern = "*";
	QuString searchPath = m_absolutePath + searchPattern;
	const QuString currRelativeDir = ".";
	const QuString previousRelativeDir = "..";

	_finddata_t currentInfo;
	intptr_t searchHandle = _findfirst(searchPath.GetRaw(), &currentInfo);

	int error = 0;
	while (!error && searchHandle != -1)
	{
		if (currentInfo.attrib == _A_HIDDEN)
		{
			goto getNextInfo;
		}

		if (currentInfo.attrib == _A_SUBDIR)
		{
			if (currRelativeDir == currentInfo.name || previousRelativeDir == currentInfo.name)
			{
				goto getNextInfo;
			}

			QuPath nextDir;
			nextDir.m_absolutePath = QuString::F("%s%s%s", m_absolutePath.GetRaw(), currentInfo.name, "\\");
			nextDir.InitializeDirectory();
			m_subDirectories.push_back(nextDir);
		}
		else
		{
			m_filesInDirectory.push_back(QuString::F("%s%s", m_absolutePath.GetRaw(), currentInfo.name));
		}

	getNextInfo:
		error = _findnext(searchHandle, &currentInfo);
	}
}
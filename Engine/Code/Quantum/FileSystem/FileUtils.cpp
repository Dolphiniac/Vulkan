#include "Quantum/FileSystem/FileUtils.h"

#include <corecrt_io.h>
#include <io.h>


//-----------------------------------------------------------------------------------------------
static void GetFilesInPath(std::vector<QuString>& outPaths, const QuString& directoryWithTrailingSlash, const QuString& searchPattern, bool shouldRecurse)
{
	_finddata_t fileInfo;
	QuString fullPath = directoryWithTrailingSlash + searchPattern;
	intptr_t fileHandle = _findfirst(fullPath.GetRaw(), &fileInfo);

	int error = 0;
	while (!error && fileHandle != -1)
	{
		if (fileInfo.attrib != _A_HIDDEN && fileInfo.attrib != _A_SUBDIR)
		{
			outPaths.push_back(directoryWithTrailingSlash + fileInfo.name);
		}
		error = _findnext(fileHandle, &fileInfo);
	}

	if (shouldRecurse)
	{
		_finddata_t dirInfo;
		QuString dirPath = directoryWithTrailingSlash + "*";
		intptr_t dirHandle = _findfirst(dirPath.GetRaw(), &dirInfo);

		error = 0;
		while (!error && dirHandle != -1)
		{
			if (dirInfo.attrib != _A_HIDDEN && dirInfo.attrib == _A_SUBDIR)
			{
				QuString dirName = dirInfo.name;
				dirName += "/";
				if (dirName != "./" && dirName != "../")
				{
					GetFilesInPath(outPaths, directoryWithTrailingSlash + dirName, searchPattern, shouldRecurse);
				}
			}
			error = _findnext(dirHandle, &dirInfo);
		}
	}
}


//-----------------------------------------------------------------------------------------------
std::vector<QuString> QuFile::GetPaths(const QuString& directoryWithTrailingSlash, const QuString& searchPattern /* = "*.*" */, bool shouldRecurse /* = true */)
{
	std::vector<QuString> results;
	GetFilesInPath(results, directoryWithTrailingSlash, searchPattern, shouldRecurse);

	return results;
}
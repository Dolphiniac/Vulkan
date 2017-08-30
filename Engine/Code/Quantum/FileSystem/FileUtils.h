#pragma once

#include "Quantum/Core/String.h"

#include <vector>


//-----------------------------------------------------------------------------------------------
namespace QuFile
{
	std::vector<QuString> GetPaths(const QuString& directoryWithTrailingSlash, const QuString& searchPattern = "*.*", bool shouldRecurse = true);
}
//-----------------------------------------------------------------------------------------------
// Based on code written by Squirrel Eiserloh
//
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Rgba.hpp"
#include <stdarg.h>


//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const int maxLength, const char* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}


//-----------------------------------------------------------------------------------------------
void ToLower(std::string& inString)
{
	for (char& c : inString)
	{
		if (c >= 'A' && c <= 'Z')
		{
			c += ('a' - 'A'); //Convert any uppercase strings to lowercase
		}
	}
}


//-----------------------------------------------------------------------------------------------
IntVector2 ToIntVector2(const std::string& inString)
{
	std::vector<std::string> strings = SplitOnDelimiter(inString, ',');
	GUARANTEE_OR_DIE(strings.size() == 2, Stringf("String %s is not a valid IntVector2", inString.c_str()));
	return IntVector2(std::stoi(strings[0]), std::stoi(strings[1]));
}


//-----------------------------------------------------------------------------------------------
std::vector<std::string> SplitOnDelimiter(const std::string& inString, char delimiter, bool trimEdgeWhitespaceCharacters)
{
	std::vector<std::string> result;
	std::string workingString = inString;
	while (!workingString.empty())
	{
		size_t offsetOfDelimiter = workingString.find(delimiter);
		std::string thisString = workingString.substr(0, offsetOfDelimiter);
		if (trimEdgeWhitespaceCharacters)
		{
			while (!thisString.empty() && (thisString[0] == '\t' || thisString[0] == '\r' || thisString[0] == '\n' || thisString[0] == ' '))
			{
				thisString = thisString.substr(1);
			}
			while (!thisString.empty() && (thisString[thisString.size() - 1] == '\t' || thisString[thisString.size() - 1] == '\r' ||
				thisString[thisString.size() - 1] == '\n' || thisString[thisString.size() - 1] == ' '))
			{
				thisString = thisString.substr(0, thisString.size() - 1);
			}
		}
		result.push_back(thisString);
		if (offsetOfDelimiter == std::string::npos)
		{
			workingString = "";
		}
		else
		{
			workingString = workingString.substr(offsetOfDelimiter + 1);
		}
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
std::vector<std::string> SplitOnDelimiters(const std::string& inString, int numDelimiters, ...)
{
	va_list delimiterList;
	va_start(delimiterList, numDelimiters);
	std::vector<std::string> delimiters;
	for (int i = 0; i < numDelimiters; i++)
	{
		delimiters.push_back(va_arg(delimiterList, const char*));
	}
	va_end(delimiterList);
	std::vector<std::string> stringPieces;

	size_t start = 0, end = 0;

	while (end != std::string::npos)
	{
		size_t shortestEnd = std::string::npos;
		std::string bestDelimiter = "";
		for (const std::string& delim : delimiters)
		{
			end = inString.find(delim, start);
			if (end < shortestEnd)
			{
				shortestEnd = end;
				bestDelimiter = delim;
			}
		}
		end = shortestEnd;
		stringPieces.push_back(inString.substr(start,
			(end == std::string::npos) ? std::string::npos : end - start));

		start = ((end >(std::string::npos - bestDelimiter.size()))
			? std::string::npos : end + bestDelimiter.size());
	}

	return stringPieces;
}


//-----------------------------------------------------------------------------------------------
void TrimBeginning(std::string& toTrim)
{
	while (!toTrim.empty())
	{
		char firstChar = toTrim[0];
		if (firstChar != ' ')
		{
			return;
		}
		else
		{
			toTrim = toTrim.substr(1);
		}
	}
}


//-----------------------------------------------------------------------------------------------
void TrimEnd(std::string& toTrim)
{
	while (!toTrim.empty())
	{
		char lastChar = toTrim[toTrim.size() - 1];
		if (lastChar != ' ')
		{
			return;
		}
		else
		{
			toTrim = toTrim.substr(0, toTrim.size() - 1);
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Trim(std::string& toTrim)
{
	TrimBeginning(toTrim);
	TrimEnd(toTrim);
}


//-----------------------------------------------------------------------------------------------
Rgba GetColorFromHexString(const std::string& hexString)
{
	std::string workingString = hexString;
	std::string errString = "Invalid hex string, must be in FFFFFF or 0xFFFFFF format";
	size_t hexLength = workingString.size();
	GUARANTEE_OR_DIE(hexLength == 6 || hexLength == 8, errString);

	if (hexLength == 8)
	{
		GUARANTEE_OR_DIE(workingString.substr(0, 2) == "0x", errString);
		workingString = workingString.substr(2);
	}
	try
	{
		Rgba result;
		result.r = (unsigned char)std::stoi(workingString.substr(0, 2), 0, 16);
		result.g = (unsigned char)std::stoi(workingString.substr(2, 2), 0, 16);
		result.b = (unsigned char)std::stoi(workingString.substr(4, 2), 0, 16);
		result.a = 255;
// 		unsigned int rgba = std::stoi(workingString, 0, 16);
// 		rgba <<= 8;
// 		rgba += 0xFF;
//		return RGBA(rgba);
		return result;
	}
	catch (const std::exception&)
	{
		ERROR_AND_DIE(errString);
	}
}


//-----------------------------------------------------------------------------------------------
Rgba ToColor(const std::string& vecString)
{
	std::vector<std::string> components = SplitOnDelimiter(vecString, ',');

	GUARANTEE_OR_DIE(components.size() == 3, Stringf("Unsupported vec string for color conversion %s", vecString.c_str()));

	unsigned char r = (unsigned char)std::stoi(components[0]);
	unsigned char g = (unsigned char)std::stoi(components[1]);
	unsigned char b = (unsigned char)std::stoi(components[2]);

	unsigned char a = 255;

	Rgba result;
	result.r = r;
	result.g = g;
	result.b = b;
	result.a = a;

	return result;
}


//-----------------------------------------------------------------------------------------------
std::string GetIndefiniteArticle(const std::string& testWord, bool capitalize /* = false */)
{
	std::string result = "";
	if (capitalize)
	{
		result.push_back('A');
	}
	else
	{
		result.push_back('a');
	}

	switch (testWord.front())
	{
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
	case 'A':
	case 'E':
	case 'I':
	case 'O':
	case 'U':
		result.push_back('n');
		break;
	}

	return result;
}
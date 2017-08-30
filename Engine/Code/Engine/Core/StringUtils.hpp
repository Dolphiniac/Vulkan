//-----------------------------------------------------------------------------------------------
// Based on code written by Squirrel Eiserloh
//
#pragma once


//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );
void ToLower(std::string& inString);
class IntVector2 ToIntVector2(const std::string& inString);
std::vector<std::string> SplitOnDelimiter(const std::string& inString, char delimiter, bool trimEdgeWhiteSpaceCharacters = true);
std::vector<std::string> SplitOnDelimiters(const std::string& inString, int numDelimiters, ...);
void TrimBeginning(std::string& toTrim);
void TrimEnd(std::string& toTrim);
void Trim(std::string& toTrim);
struct Rgba GetColorFromHexString(const std::string& hexString);
struct Rgba ToColor(const std::string& vecString);

//Only works for voweled words,  words like honor will still give 'a' instead of 'an'
std::string GetIndefiniteArticle(const std::string& testWord, bool capitalize = false);
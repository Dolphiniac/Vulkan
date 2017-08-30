#pragma once

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Rgba.hpp"

#include <string>
#include <deque>
#include <map>


//-----------------------------------------------------------------------------------------------
class ConsoleCommand;


//-----------------------------------------------------------------------------------------------
typedef void(*CommandFunc)(ConsoleCommand&);


//-----------------------------------------------------------------------------------------------
typedef void(*ConsolePrintFunc)(const std::string& toPrint, const Rgba& color);
typedef void(*ConsolePrintfFunc)(const Rgba& color, const char* format, ...);


//-----------------------------------------------------------------------------------------------
extern ConsolePrintFunc ConsolePrint;
extern ConsolePrintfFunc ConsolePrintf;


//-----------------------------------------------------------------------------------------------
class ConsoleCommand
{
public:
	ConsoleCommand(const std::string& args);
	std::string GetFuncName() const { return m_funcName; }
	inline bool CallFunc();
	inline std::string GetNextArg();
	inline std::string GetArgString() const;
	inline std::string GetRemainingArgString() const;
	inline static void RegisterCommand(const std::string& text, CommandFunc commandCB);

private:
	std::string m_allArgs;
	std::deque<std::string> m_args;
	static std::map<std::string, CommandFunc>* s_commandRegistry;
	std::string m_funcName;
};


//-----------------------------------------------------------------------------------------------
class CommandRegisterHelper
{
public:
	CommandRegisterHelper(const std::string& text, CommandFunc commandCB)
	{
		std::string command(text);
		ToLower(command);
		ConsoleCommand::RegisterCommand(command, commandCB);
	}
};


//Register a command with the console------------------------------------------------------------
#define CONSOLE_COMMAND(name, args) void CommandFunc_ ## name ## (ConsoleCommand&); \
static CommandRegisterHelper CommandHelper_ ## name ## (#name, CommandFunc_ ## name); \
void CommandFunc_ ## name ## (ConsoleCommand& args)
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
std::string ConsoleCommand::GetNextArg()
{
	if (m_args.empty())
		return "";


	std::string result = m_args.back();
	m_args.pop_back();
	return result;
}


//-----------------------------------------------------------------------------------------------
bool ConsoleCommand::CallFunc()
{
	auto funcIter = s_commandRegistry->find(m_funcName);
	if (funcIter == s_commandRegistry->end())
		return false;

	funcIter->second(*this);
	return true;
}


//-----------------------------------------------------------------------------------------------
std::string ConsoleCommand::GetArgString() const
{
	std::string untrimmedArgs = m_allArgs.substr(m_funcName.size());

	Trim(untrimmedArgs);

	return untrimmedArgs;
}


//-----------------------------------------------------------------------------------------------
std::string ConsoleCommand::GetRemainingArgString() const
{
	std::string result = "";

	for (auto iter = m_args.rbegin(); iter != m_args.rend(); iter++)
	{
		result += *iter + " ";
	}

	Trim(result);

	return result;
}


//-----------------------------------------------------------------------------------------------
void ConsoleCommand::RegisterCommand(const std::string& text, CommandFunc commandCB)
{
	if (!s_commandRegistry)
	{
		s_commandRegistry = new std::map<std::string, CommandFunc>();
	}
	s_commandRegistry->insert(std::make_pair(text, commandCB)); 
}
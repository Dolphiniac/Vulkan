#include "Engine/Core/ConsoleCommand.hpp"

#include <sstream>


//-----------------------------------------------------------------------------------------------
ConsolePrintFunc ConsolePrint = nullptr;
ConsolePrintfFunc ConsolePrintf = nullptr;


//-----------------------------------------------------------------------------------------------
std::map<std::string, CommandFunc>* ConsoleCommand::s_commandRegistry = nullptr;


ConsoleCommand::ConsoleCommand(const std::string& args)
	: m_allArgs(args)
	, m_args()
{
	ToLower(m_allArgs);

	std::stringstream argStream;
	std::string currArg;
	argStream << m_allArgs;
	while (std::getline(argStream, currArg, ' '))
	{
		if (currArg != "")
		{
			Trim(currArg);
			m_args.push_front(currArg);
		}
	}
	m_funcName = m_args.back();
	m_args.pop_back();
}
#include "Game/ConsoleCommand.hpp"

#include <sstream>


//-----------------------------------------------------------------------------------------------
std::map<std::string, CommandFunc> ConsoleCommand::s_commandRegistry;


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
		m_args.push_front(currArg);
	}
	m_funcName = m_args.back();
	m_args.pop_back();
}
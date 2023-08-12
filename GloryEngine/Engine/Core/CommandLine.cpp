#include "CommandLine.h"

namespace Glory
{
	CommandLine::CommandLine(int argc, char* argv[])
	{
		for (size_t i = 0; i < argc; i++)
		{
			std::string argName = "";
			std::string argValue = "";

			std::string arg = argv[i];
			if (arg[0] != '-') continue;

			argName = arg;
			size_t assignmentOpIndex = arg.find('=');
			if (assignmentOpIndex != std::string::npos)
			{
				argName = arg.substr(1, assignmentOpIndex - 1);
				argValue = arg.substr(assignmentOpIndex + 1);
			}
			m_Args[argName] = argValue;
		}
	}

	CommandLine::~CommandLine()
	{
		m_Args.clear();
	}

	bool CommandLine::Contains(const std::string& arg) const
	{
		return m_Args.find(arg) != m_Args.end();
	}

	bool CommandLine::GetValue(const std::string& arg, std::string& value) const
	{
		value = "";
		if (!Contains(arg)) return false;
		value = m_Args.at(arg);
		return true;
	}

	size_t CommandLine::ArgCount() const
	{
		return m_Args.size();
	}
}

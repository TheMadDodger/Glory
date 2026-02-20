#pragma once
#include <map>
#include <string>

namespace Glory::Utils
{
	class CommandLine
	{
	public:
		CommandLine(int argc, char* argv[]);
		virtual ~CommandLine();

		bool Contains(const std::string& arg) const;
		bool GetValue(const std::string& arg, std::string& value) const;
		size_t ArgCount() const;

	private:
		std::map<std::string, std::string> m_Args;
	};
}

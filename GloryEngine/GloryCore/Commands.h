#pragma once
#include <typeinfo>
#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include "Debug.h"

namespace Glory
{
	class BaseParser
	{
	public:
		virtual const std::type_info& GetBaseType() { return typeid(BaseParser); }
		virtual const std::type_info& GetType() = 0;
	};

	template<typename T>
	class TemplatedParser : public BaseParser
	{
	public:
		virtual T Parse(const std::string& data, bool& succes) = 0;
		const std::type_info& GetType() override
		{
			return typeid(T);
		}
	};

	class IntParser : public TemplatedParser<int>
	{
		int Parse(const std::string& data, bool& succes) override
		{
			try
			{
				return std::stoi(data.data());
			}
			catch (const std::exception&)
			{
				succes = false;
				Debug::LogNotice(">>>> Invalid Argument", false);
				return -1;
			}
		}
	};

	class FloatParser : public TemplatedParser<float>
	{
		float Parse(const std::string& data, bool& succes) override
		{
			try
			{
				return std::stof(data.data());
			}
			catch (const std::exception&)
			{
				succes = false;
				Debug::LogNotice(">>>> Invalid Argument", false);
				return -1.0f;
			}
		}
	};

	class StringParser : public TemplatedParser<std::string>
	{
		std::string Parse(const std::string& data, bool& succes) override
		{
			try
			{
				return data;
			}
			catch (const std::exception&)
			{
				succes = false;
				Debug::LogNotice(">>>> Invalid Argument", false);
				return "";
			}
		}
	};

	class SizeTParser : public TemplatedParser<size_t>
	{
		size_t Parse(const std::string& data, bool& succes) override
		{
			try
			{
				size_t value;
				std::stringstream sstream = std::stringstream();
				sstream << data;
				sstream >> value;

				return value;
			}
			catch (const std::exception&)
			{
				succes = false;
				Debug::LogNotice(">>>> Invalid Argument", false);
				return 0;
			}
		}
	};

	class Parser
	{
	public:
		template<typename T>
		static bool Parse(const std::string& data, T& dataOut)
		{
			for (auto parser : m_Parsers)
			{
				if (parser->GetType() == typeid(T))
				{
					TemplatedParser<T>* newParser = dynamic_cast<TemplatedParser<T>*>(parser);

					bool succes = true;
					dataOut = newParser->Parse(data, succes);
					return succes;
				}
			}
		}

		static void Destroy()
		{
			for (auto pParser : m_Parsers)
			{
				delete pParser;
			}
			m_Parsers.clear();
		}

	private:
		static std::vector<BaseParser*> m_Parsers;
	};

	class BaseConsoleCommand
	{
	public:
		BaseConsoleCommand(const std::string& command) : m_Command(command) {}
		~BaseConsoleCommand() {}

		const std::string& GetCommand()
		{
			return m_Command;
		}

	protected:
		virtual bool RootExecuteCommand(const std::vector<std::string>& args) = 0;

	private:
		friend class Console;
		const std::string m_Command;
	};

	class ConsoleCommand : public BaseConsoleCommand
	{
	public:
		ConsoleCommand(const std::string& command, std::function<bool()> func) : BaseConsoleCommand(command), m_Function(func) {}
		virtual ~ConsoleCommand() {}

	private:
		virtual bool RootExecuteCommand(const std::vector<std::string>&) override
		{
			return m_Function();
		}

		std::function<bool()> m_Function;
	};


	template<typename A1>
	class ConsoleCommand1 : public BaseConsoleCommand
	{
	public:
		ConsoleCommand1(const std::string& command, std::function<bool(A1)> func) : BaseConsoleCommand(command), m_Function(func) {}
		virtual ~ConsoleCommand1() {}

	private:
		virtual bool RootExecuteCommand(const std::vector<std::string>& args) override
		{
			if (!UnpackArguments(args))
			{
				Debug::LogNotice(">>>> Invalid Argument count", false);
				return true;
			}
			return m_Function(m_Argument1);
		}

		bool UnpackArguments(const std::vector<std::string>& arguments)
		{
			if (arguments.size() <= 0) return false;

			if (!Parser::Parse<A1>(arguments[0], m_Argument1)) return false;
			return true;
		}

	private:
		A1 m_Argument1;
		std::function<bool(A1)> m_Function;
	};

	template<typename A1, typename A2>
	class ConsoleCommand2 : public BaseConsoleCommand
	{
	public:
		ConsoleCommand2(const std::string& command, std::function<bool(A1, A2)> func) : BaseConsoleCommand(command), m_Function(func) {}
		virtual ~ConsoleCommand2() {}

	private:
		virtual bool RootExecuteCommand(const std::vector<std::string>& args) override
		{
			if (!UnpackArguments(args))
			{
				Debug::LogNotice(">>>> Invalid Argument count!", false);
				return true;
			}
			return m_Function(m_Argument1, m_Argument2);
		}

		bool UnpackArguments(const std::vector<std::string>& arguments)
		{
			if (arguments.size() <= 1) return false;

			if (!Parser::Parse<A1>(arguments[0], m_Argument1)) return false;
			if (!Parser::Parse<A2>(arguments[1], m_Argument2)) return false;
			return true;
		}

	private:
		A1 m_Argument1;
		A2 m_Argument2;
		std::function<bool(A1, A2)> m_Function;
	};

	template<typename A1, typename A2, typename A3>
	class ConsoleCommand3 : public BaseConsoleCommand
	{
	public:
		ConsoleCommand3(const std::string& command, std::function<bool(A1, A2, A3)> func) : BaseConsoleCommand(command), m_Function(func) {}
		virtual ~ConsoleCommand3() {}

	private:
		virtual bool RootExecuteCommand(const std::vector<std::string>& args) override
		{
			if (!UnpackArguments(args))
			{
				Debug::LogNotice(">>>> Invalid Argument count!", false);
				return true;
			}
			return m_Function(m_Argument1, m_Argument2, m_Argument3);
		}

		bool UnpackArguments(const std::vector<std::string>& arguments)
		{
			if (arguments.size() <= 2) return false;

			if (!Parser::Parse<A1>(arguments[0], m_Argument1)) return false;
			if (!Parser::Parse<A2>(arguments[1], m_Argument2)) return false;
			if (!Parser::Parse<A3>(arguments[2], m_Argument3)) return false;
			return true;
		}

	private:
		A1 m_Argument1;
		A2 m_Argument2;
		A3 m_Argument3;
		std::function<bool(A1, A2, A3)> m_Function;
	};

	template<typename A1, typename A2, typename A3, typename A4>
	class ConsoleCommand4 : public BaseConsoleCommand
	{
	public:
		ConsoleCommand4(const std::string& command, std::function<bool(A1, A2, A3, A4)> func) : BaseConsoleCommand(command), m_Function(func) {}
		virtual ~ConsoleCommand4() {}

	private:
		virtual bool RootExecuteCommand(const std::vector<std::string>& args) override
		{
			if (!UnpackArguments(args))
			{
				Debug::LogNotice(">>>> Invalid Argument count!", false);
				return true;
			}
			return m_Function(m_Argument1, m_Argument2, m_Argument3, m_Argument4);
		}

		bool UnpackArguments(const std::vector<std::string>& arguments)
		{
			if (arguments.size() <= 3) return false;

			if (!Parser::Parse<A1>(arguments[0], m_Argument1)) return false;
			if (!Parser::Parse<A2>(arguments[1], m_Argument2)) return false;
			if (!Parser::Parse<A3>(arguments[2], m_Argument3)) return false;
			if (!Parser::Parse<A4>(arguments[3], m_Argument4)) return false;
			return true;
		}

	private:
		A1 m_Argument1;
		A2 m_Argument2;
		A3 m_Argument3;
		A4 m_Argument4;
		std::function<bool(A1, A2, A3, A4)> m_Function;
	};

	template<typename A1, typename A2, typename A3, typename A4, typename A5>
	class ConsoleCommand5 : public BaseConsoleCommand
	{
	public:
		ConsoleCommand5(const std::string& command, std::function<bool(A1, A2, A3, A4, A5)> func) : BaseConsoleCommand(command), m_Function(func) {}
		virtual ~ConsoleCommand5() {}

	private:
		virtual bool RootExecuteCommand(const std::vector<std::string>& args) override
		{
			if (!UnpackArguments(args))
			{
				Debug::LogNotice(">>>> Invalid Argument count!", false);
				return true;
			}
			return m_Function(m_Argument1, m_Argument2, m_Argument3, m_Argument4);
		}

		bool UnpackArguments(const std::vector<std::string>& arguments)
		{
			if (arguments.size() <= 4) return false;

			if (!Parser::Parse<A1>(arguments[0], m_Argument1)) return false;
			if (!Parser::Parse<A2>(arguments[1], m_Argument2)) return false;
			if (!Parser::Parse<A3>(arguments[2], m_Argument3)) return false;
			if (!Parser::Parse<A4>(arguments[3], m_Argument4)) return false;
			if (!Parser::Parse<A5>(arguments[4], m_Argument5)) return false;
			return true;
		}

	private:
		A1 m_Argument1;
		A2 m_Argument2;
		A3 m_Argument3;
		A4 m_Argument4;
		A5 m_Argument5;
		std::function<bool(A1, A2, A3, A4, A5)> m_Function;
	};
}
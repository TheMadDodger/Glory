#include "Tester.h"
#include "ConsoleColors.h"

#include <iostream>
#include <print>
#include <cassert>
#include <typeindex>
#include <filesystem>

namespace Glory::Utils
{
	struct Exception {};

	Tester::Tester(): m_pCommandLine(nullptr), m_pCurrentTest(nullptr), m_Verbose(false)
	{
	}

	Tester::~Tester()
	{
		m_Tests.clear();
		m_pCommandLine = nullptr;
		m_pCurrentTest = nullptr;
	}

	int Tester::operator()()
	{
		m_Verbose = m_pCommandLine->Contains("verbose");

		m_State.m_CheckCounter = 0;
		m_State.m_TestCounter = 0;
		m_State.m_ErrorCounter = 0;
		m_State.m_WarnCounter = 0;

		std::println(CONSOLE_WHITE(Starting {} with {} test cases...), m_Testname, m_Tests.size());

		for (Test& testCase : m_Tests)
		{
			++m_State.m_TestCounter;
			const uint32_t lastCheckCount = m_State.m_CheckCounter;

			m_pCurrentTest = &testCase;
			(this->*testCase.Initilizer)();
			bool fail = false;
			try
			{
				(this->*testCase.Test)();
			}
			catch (const Exception&)
			{
				++m_State.m_ErrorCounter;
				fail = true;
			}
			(this->*testCase.Cleanup)();
			if (!fail && lastCheckCount == m_State.m_CheckCounter)
			{
				CONSOLE_INDENT(2);
				std::println(CONSOLE_YELLOW(WARN) " " CONSOLE_MAGENTA([) CONSOLE_CYAN({}) CONSOLE_MAGENTA(])
					" This test has no checks.",
					m_State.m_TestCounter, m_State.m_CurrentFunction);
				++m_State.m_WarnCounter;
			}
			else if (!fail)
			{
				CONSOLE_INDENT(2);
				std::println(CONSOLE_GREEN(PASS) " " CONSOLE_MAGENTA([) CONSOLE_CYAN({}) CONSOLE_MAGENTA(]) " {}",
					m_State.m_TestCounter, m_State.m_CurrentFunction);
			}
		}

		if (m_State.m_ErrorCounter == 0)
			std::println(CONSOLE_WHITE(Finished {} with) " " CONSOLE_GREEN({} errors) " out of {} checks.",
				m_Testname, m_State.m_ErrorCounter, m_State.m_CheckCounter);
		else
			std::println(CONSOLE_WHITE(Finished {} with) " " CONSOLE_RED({} errors) " out of {} checks.",
				m_Testname, m_State.m_ErrorCounter, m_State.m_CheckCounter);

		return int(m_State.m_ErrorCounter);
	}

	void Tester::RegisterTest(CommandLine* pCommandLine, std::string_view filename, std::string_view testname)
	{
		m_pCommandLine = pCommandLine;
		m_Filename = filename;
		m_Testname = testname;
	}

	void Tester::SetState(const std::source_location& source)
	{
		m_State.m_CurrentFunction = GetFunctionName(source);
		m_State.m_CurrentLine = source.line();
	}

	uint8_t NumDigits(uint32_t x)
	{
		if (x >= 100000) return 6;
		if (x >= 10000) return 5;
		if (x >= 1000) return 4;
		if (x >= 100) return 3;
		if (x >= 10) return 2;
		return 1;
	}

	void Tester::VerifyInternal(const char* expression, bool result)
	{
		assert(m_pCurrentTest != nullptr);

		++m_State.m_CheckCounter;

		if (result)
		{
			LogCheck(expression);
			return;
		}
		CONSOLE_INDENT(2);
		std::println(CONSOLE_RED(FAIL) " " CONSOLE_MAGENTA([) CONSOLE_CYAN({}) CONSOLE_MAGENTA(]) " {} at {}:{}", m_State.m_TestCounter,
			m_State.m_CurrentFunction, m_Filename, m_State.m_CurrentLine);

		CONSOLE_INDENT(10 + NumDigits(m_State.m_TestCounter));
		std::println("Expression \"{}\" failed.", expression);
		throw Exception();
	}

	void Tester::CompareInternal(const char* comparator, std::expected<std::string, std::string>& result)
	{
		assert(m_pCurrentTest != nullptr);

		++m_State.m_CheckCounter;

		if (result)
		{
			LogCheck(*result);
			return;
		}
		CONSOLE_INDENT(2);
		std::println(CONSOLE_RED(FAIL) " " CONSOLE_MAGENTA([) CONSOLE_CYAN({}) CONSOLE_MAGENTA(]) " {} at {}:{}", m_State.m_TestCounter,
			m_State.m_CurrentFunction, m_Filename, m_State.m_CurrentLine);

		CONSOLE_INDENT(10 + NumDigits(m_State.m_TestCounter));
		std::println("Comparison \"{}\" failed.", comparator);
		CONSOLE_INDENT(10 + NumDigits(m_State.m_TestCounter));
		std::println("{}", result.error());
		throw Exception();
	}

	void Tester::AddTestCase(Test&& testCase)
	{
		m_Tests.emplace_back(std::move(testCase));
	}

	std::string_view Tester::GetFunctionName(const std::source_location& source)
	{
		std::string_view functionName = source.function_name();
		const size_t functionStart = functionName.rfind("::");
		if (functionStart != std::string::npos)
			functionName = functionName.substr(functionStart + 2);
		return functionName;
	}

	void Tester::LogCheck(std::string_view text)
	{
		if (!m_Verbose) return;
		std::println(CONSOLE_COLOR_START(35) "{}:{} {}" CONSOLE_COLOR_END, m_Filename, m_State.m_CurrentLine, text);
	}
}

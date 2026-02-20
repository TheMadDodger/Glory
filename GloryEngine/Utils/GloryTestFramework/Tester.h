#pragma once
#include "TestMacros.h"

#include <CommandLine.h>
#include <initializer_list>
#include <vector>

namespace Glory::Utils
{
	class CommandLine;

	/** @brief Base class for testing */
	class Tester
	{
	private:
		/** @brief Test */
		struct Test
		{
			typedef void (Tester::*Function)();

			Function Test;
			Function Initilizer;
			Function Cleanup;
		};

	public:
		/** @brief Constructor */
		Tester();
		/** @brief Destructor */
		virtual ~Tester();

		/** @brief Runs all tests and returns number of errors. */
		int operator()();

		/**
		 * @brief Register test with filename and name.
		 * @param pCommandLine Pointer to command line
		 * @param filename CPP filename that contains the tests
		 * @param testname Name of the test
		 */
		void RegisterTest(CommandLine* pCommandLine, std::string_view filename, std::string_view testname);

		/**
		 * @brief Add tests.
		 * @param T Derived class.
		 * @param tests Test functions.
		 * @param initializer Initializer function that gets called right before each test.
		 * @param cleanup Cleanup function that gets called after each test.
		 */
		template<class T>
		void AddTests(std::initializer_list<void(T::*)()> tests,
			void(T::*initializer)(), void(T::*cleanup)())
		{
			for (auto test : tests)
				AddTestCase({ static_cast<Test::Function>(test),
					initializer ? static_cast<Test::Function>(initializer) : NULL,
					cleanup ? static_cast<Test::Function>(cleanup) : NULL });
		}

		/** @overload Adds tests without initializer or cleanup. */
		template<class T>
		void AddTests(std::initializer_list<void(T::*)()> tests)
		{
			AddTests(tests, NULL, NULL);
		}

	protected:
		/**
		 * @brief Set current test state to source location.
		 * @param source Current source location of test.
		 */
		void SetState(const std::source_location& source);
		/**
		 * @brief Verify an expressions result and report it.
		 * @param expression Expression string.
		 * @param result Result of the expression.
		 *
		 * If result is false, this method will report a fail and throw.
		 */
		void VerifyInternal(const char* expression, bool result);
		/**
		 * @brief Check the result of a comparison.
		 * @param comparator Name of the comparator.
		 * @param result Result of the comparison.
		 *
		 * If comparison failed, this method will report the eror and throw.
		 */
		void CompareInternal(const char* comparator, std::expected<std::string, std::string>& result);

	private:
		std::string_view m_Filename;
		std::string_view m_Testname;

		std::vector<Test> m_Tests;
		CommandLine* m_pCommandLine;
		Test* m_pCurrentTest;
		bool m_Verbose;

		struct
		{
			std::string_view m_CurrentFunction;
			size_t m_CurrentLine = 0;

			uint32_t m_CheckCounter = 0;
			uint32_t m_TestCounter = 0;
			uint32_t m_ErrorCounter = 0;
			uint32_t m_WarnCounter = 0;
		} m_State;

	private:
		void AddTestCase(Test&& testCase);
		std::string_view GetFunctionName(const std::source_location& source);
		void LogCheck(std::string_view text);
	};
}

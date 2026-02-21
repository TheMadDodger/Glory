#pragma once
#include "TestComparators.h"

#include <source_location>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

/**
 * @brief Create main function for test application.
 * @param tester Class deriving from @ref Glory::Utils::Tester.
 */
#define GLORY_TEST_MAIN(tester) int main(int argc, char* argv[])		\
{																		\
	Glory::Utils::CommandLine commandLine(argc, argv);					\
	tester test;														\
	const char* testName = #tester;										\
	test.RegisterTest(&commandLine, __FILE__, testName);				\
	return test();														\
}

/** @brief Set current state of test to current source location */
#define GLORY_TEST_SET_STATE											\
	const std::source_location source = std::source_location::current();\
	SetState(source);

/** @brief Set current state of test to current source location and expect next expression to fail */
#define GLORY_TEST_SET_STATE_FAIL										\
	const std::source_location source = std::source_location::current();\
	SetState(source, true);

/**
 * @brief Verify if an expression is true.
 * @param expression Expression to check.
 */
#define GLORY_TEST_VERIFY(expression) do {								\
	GLORY_TEST_SET_STATE												\
	VerifyInternal(#expression, expression);							\
} while (false);

 /**
  * @brief Verify if an expression fails.
  * @param expression Expression to check.
  */
#define GLORY_TEST_FAIL(expression) do {								\
	GLORY_TEST_SET_STATE_FAIL											\
	VerifyInternal(#expression, expression);							\
} while (false);

/**
* @brief Compare 2 values using a custom comparator.
* @param a Input value.
* @param b Reference value.
* @param comparator Comparator class.
*/
#define GLORY_TEST_COMPARE_CUSTOM(a, b, comparator) do {				\
	GLORY_TEST_SET_STATE												\
	auto result = comparator()(a, b);									\
	CompareInternal(STRINGIFY(a and b using comparator), result);		\
} while (false);

/**
* @brief Compare 2 values using @ref Glory::Utils::CompareEqual.
* @param a Input value.
* @param b Reference value.
*/
#define GLORY_TEST_COMPARE(a, b)										\
GLORY_TEST_COMPARE_CUSTOM(a, b, Glory::Utils::CompareEqual)

/**
* @brief Compare 2 vector containers using @ref Glory::Utils::CompareVectors.
* @param a Input vector.
* @param b Reference vector.
* @param comparator Comparator class to compare each element with.
*/
#define GLORY_TEST_COMPARE_VECTORS_CUSTOM(a, b, comparer)				\
GLORY_TEST_COMPARE_CUSTOM(a, b, Glory::Utils::CompareVectors<comparer>)

/**
* @brief Compare 2 vector containers using @ref Glory::Utils::CompareVectors
*        and @ref Glory::Utils::CompareEqual for each element.
* @param a Input vector.
* @param b Reference vector.
*/
#define GLORY_TEST_COMPARE_VECTORS(a, b)								\
GLORY_TEST_COMPARE_VECTORS_CUSTOM(a, b, Glory::Utils::CompareEqual)

/**
* @brief Check if an expression throws a specific exception.
* @param expression Expression to check.
* @param exception The expected exception.
*/
#define GLORY_TEST_THROW(expression, exception) do {					\
	GLORY_TEST_SET_STATE												\
	bool raised = false;												\
	try																	\
	{																	\
		expression;														\
	}																	\
	catch (const exception&)											\
	{																	\
		raised = true;													\
	}																	\
	ExpectThrowInternal(#expression, raised);							\
} while (false);
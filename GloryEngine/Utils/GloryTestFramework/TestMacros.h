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
#define GLORY_TEST_GET_STATE											\
	const std::source_location source = std::source_location::current();\
	SetState(source);

/**
 * @brief Verify if an expression is true.
 * @param expression Expression to check.
 */
#define GLORY_TEST_VERIFY(expression) do {								\
	GLORY_TEST_GET_STATE												\
	VerifyInternal(#expression, expression);							\
} while (false);

/**
* @brief Compare 2 values using a custom comparator.
* @param a Input value.
* @param b Reference value.
* @param comparator Comparator class.
*/
#define GLORY_TEST_COMPARE_CUSTOM(a, b, comparator) do {				\
	GLORY_TEST_GET_STATE												\
	auto result = comparator()(a, b);									\
	CompareInternal(#comparator, result);								\
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
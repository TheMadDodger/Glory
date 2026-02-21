#pragma once
#include <expected>
#include <string>
#include <format>
#include <vector>

namespace Glory::Utils
{
	template<typename T>
	concept Comparable = requires (const T a, const T b) {
		{ a == b } -> std::same_as<bool>;
		{ a >  b } -> std::same_as<bool>;
		{ a <  b } -> std::same_as<bool>;
		{ a >= b } -> std::same_as<bool>;
		{ a <= b } -> std::same_as<bool>;
	};

	template<typename T>
	concept Formattable = std::formattable<T, char>;

	/** @brief Compare an input to an expected value */
	struct CompareEqual
	{
		template<typename T> requires Comparable<T> && Formattable<T>
		std::expected<std::string, std::string> operator()(const T& a, const T& b)
		{
			if (a == b) return std::format("{} == {}", a, b);
			return std::unexpected(std::format("Expected {} to equal {}.", a, b));
		}
	};

	/** @brief Compare if an input is greater than a reference value */
	struct CompareGreater
	{
		template<typename T> requires Comparable<T>&& Formattable<T>
		std::expected<std::string, std::string> operator()(const T& a, const T& b)
		{
			if (a > b) return std::format("{} > {}", a, b);
			return std::unexpected(std::format("Expected {} to be greater than {}.", a, b));
		}
	};

	/** @brief Compare if an input is less than a reference value */
	struct CompareLess
	{
		template<typename T> requires Comparable<T>&& Formattable<T>
		std::expected<std::string, std::string> operator()(const T& a, const T& b)
		{
			if (a < b) return std::format("{} < {}", a, b);
			return std::unexpected(std::format("Expected {} to be less than {}.", a, b));
		}
	};

	/** @brief Compare if an input is greater than or equal to a reference value */
	struct CompareGreaterOrEqual
	{
		template<typename T> requires Comparable<T>&& Formattable<T>
		std::expected<std::string, std::string> operator()(const T& a, const T& b)
		{
			if (a >= b) return std::format("{} >= {}", a, b);
			return std::unexpected(std::format("Expected {} to be greater than or equal to {}.", a, b));
		}
	};

	/** @brief Compare if an input is less than or equal to a reference value */
	struct CompareLessOrEqual
	{
		template<typename T> requires Comparable<T> && Formattable<T>
		std::expected<std::string, std::string> operator()(const T& a, const T& b)
		{
			if (a <= b) return std::format("{} <= {}", a, b);
			return std::unexpected(std::format("Expected {} to be less than or equal to {}.", a, b));
		}
	};

	/**
	 * @brief Compare if 2 vector containers are equal
	 * First compares the size then uses Comp to compare each element
	 */
	template<typename Comp>
	struct CompareVectors
	{
		template<typename E> requires Formattable<E>
		std::expected<std::string, std::string> operator()(const std::vector<E>& a, const std::vector<E>& b)
		{
			if (a.size() != b.size())
				return std::unexpected(std::format("Compared containers do not have the same size.", a, b));

			auto elementComparer = Comp();
			for (size_t i = 0; i < a.size(); ++i)
			{
				auto result = elementComparer(a[i], b[i]);
				if (!result)
					return std::unexpected(
						std::format("Element at index {}: {}", i, result.error()));
			}

			return "Containers are equal";
		}
	};
}
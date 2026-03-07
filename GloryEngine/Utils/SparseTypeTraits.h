#pragma once
#include <type_traits>
#include <concepts>

namespace Glory::Utils::ECS
{
	template<typename ID>
	concept SparseCompatibleSparse = !std::equality_comparable<ID> &&
		std::convertible_to<ID, size_t>&& requires (ID a, ID b) {
			{ a == b } -> std::same_as<bool>;
			{ a > b } -> std::same_as<bool>;
			{ a < b } -> std::same_as<bool>;
			{ a >= b } -> std::same_as<bool>;
			{ a <= b } -> std::same_as<bool>;
			{ a - b } -> std::same_as<ID>;
			{ a + b } -> std::same_as<ID>;
			{ a + 1 } -> std::same_as<ID>;
			{ a - 1 } -> std::same_as<ID>;
			{ a++ };
			{ ++a };
			{ a++ };
			{ --a };
	};

	template<typename Element>
	concept SparseCompatibleDense = !std::copyable<Element> && std::movable<Element>;

	template<typename ID, typename Element>
	concept SparseCompatible = requires {
		SparseCompatibleSparse<ID>;
		SparseCompatibleDense<Element>;
	};
}
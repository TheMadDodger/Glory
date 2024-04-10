#pragma once
#include <vector>

namespace Glory::Utils::Reflect
{
	template<typename T>
	struct BasicBuffer
	{
		std::vector<T> m_Buffer;
	};

	struct BinaryBuffer : public BasicBuffer<char> {};
}

#pragma once
#include <cstdint>
#include <cstring>

namespace Glory::Utils
{
	struct BitSet
	{
	private:
		using Element = uint32_t;
	public:
		BitSet(size_t capacity = 32, bool defaultOn = false);
		~BitSet();

	public:
		void Set(Element index);
		void UnSet(Element index);
		bool IsSet(Element index);
		void Reserve(size_t capacity);

	private:
		Element* m_pMemory = nullptr;
		size_t m_Capacity;
	};
}

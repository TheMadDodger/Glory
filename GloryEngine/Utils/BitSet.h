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
		void Set(Element index, bool on);
		void UnSet(Element index);
		void Reserve(size_t capacity);
		bool IsSet(Element index) const;

	private:
		Element* m_pMemory = nullptr;
		size_t m_Capacity;
	};
}

#pragma once
#include <cstdint>

namespace Glory::Utils
{
	struct BitSet
	{
	private:
		using Element = uint32_t;
	public:
		BitSet(size_t capacity = 32, bool defaultOn = false);
		BitSet(BitSet&& other) noexcept;
		BitSet(const BitSet& other);
		~BitSet();
		void operator=(BitSet&& other) noexcept;
		void operator=(const BitSet& other) noexcept;

	public:
		void Set(Element index);
		void Toggle(Element index);
		void Set(Element index, bool on);
		void SetAll();
		void UnSet(Element index);
		void Clear();
		void Reserve(size_t capacity);
		bool IsSet(Element index) const;
		bool HasAnySet() const;

		Element* Data() const;
		size_t DataSize() const;

	private:
		Element* m_pMemory = nullptr;
		size_t m_Capacity;
	};
}

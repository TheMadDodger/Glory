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
		void Set(size_t index);
		void Toggle(size_t index);
		void Set(size_t index, bool on);
		void SetAll();
		void UnSet(size_t index);
		void Clear();
		void Reserve(size_t capacity);
		bool IsSet(size_t index) const;
		bool HasAnySet() const;

		Element* Data() const;
		size_t DataSize() const;

	private:
		Element* m_pMemory = nullptr;
		size_t m_Capacity;
	};
}

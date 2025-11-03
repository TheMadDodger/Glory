#include "BitSet.h"

#include <cstring>

namespace Glory::Utils
{
	BitSet::BitSet(size_t capacity, bool defaultOn) : m_pMemory(new Element[capacity + (32 - capacity % 32)]), m_Capacity(capacity) {
		for (size_t i = 0; i < capacity; ++i)
		{
			m_pMemory[i] = defaultOn ? 0xFFFFFFFF : 0;
		}
	}

	BitSet::BitSet(BitSet&& other) noexcept: m_pMemory(other.m_pMemory), m_Capacity(other.m_Capacity)
	{
		other.m_pMemory = nullptr;
		other.m_Capacity = 0;
	}

	BitSet::BitSet(const BitSet& other)
	{
		Reserve(other.m_Capacity);
		std::memcpy(m_pMemory, other.m_pMemory, other.DataSize());
	}

	BitSet::~BitSet()
	{
		if (!m_pMemory) return;
		delete[] m_pMemory;
		m_pMemory = nullptr;

		m_Capacity = 0;
	}

	void BitSet::operator=(BitSet&& other) noexcept
	{
		if (m_pMemory)
		{
			delete[] m_pMemory;
			m_pMemory = nullptr;
		}

		m_pMemory = other.m_pMemory;
		m_Capacity = other.m_Capacity;

		other.m_pMemory = nullptr;
		other.m_Capacity = 0;
	}

	void BitSet::operator=(const BitSet& other) noexcept
	{
		Reserve(other.m_Capacity);
		std::memcpy(m_pMemory, other.m_pMemory, other.DataSize());
	}

	void BitSet::Set(Element index)
	{
		const Element elementIndex = index / (sizeof(Element) * 8);
		const Element bitIndex = index - elementIndex * sizeof(Element) * 8;
		m_pMemory[elementIndex] |= 1 << bitIndex;
	}

	void BitSet::Toggle(Element index)
	{
		if (IsSet(index))
		{
			UnSet(index);
			return;
		}
		Set(index);
	}

	void BitSet::Set(Element index, bool on)
	{
		if (on)
		{
			Set(index);
			return;
		}
		UnSet(index);
	}

	void BitSet::SetAll()
	{
		std::memset(m_pMemory, ~0, m_Capacity);
	}

	void BitSet::UnSet(Element index)
	{
		const Element elementIndex = index / (sizeof(Element) * 8);
		const uint32_t bitIndex = index - elementIndex * sizeof(Element) * 8;
		m_pMemory[elementIndex] &= ~(1 << bitIndex);
	}

	void BitSet::Clear()
	{
		std::memset(m_pMemory, 0, m_Capacity);
	}

	void BitSet::Reserve(size_t capacity)
	{
		if (m_Capacity >= capacity) return;
		const size_t newCapacity = capacity + (32 - capacity % 32);
		Element* pNewMemory = new Element[newCapacity];
		std::memcpy(pNewMemory, m_pMemory, m_Capacity);
		m_Capacity = newCapacity;
		delete[] m_pMemory;
		m_pMemory = pNewMemory;
	}

	bool BitSet::IsSet(Element index) const
	{
		const Element elementIndex = index / (sizeof(Element) * 8);
		const uint32_t bitIndex = index - elementIndex * sizeof(Element) * 8;
		return m_pMemory[elementIndex] & 1 << bitIndex;
	}

	bool BitSet::HasAnySet() const
	{
		for (size_t i = 0; i < m_Capacity/32; ++i)
		{
			if (m_pMemory[i] == 0) continue;
			return true;
		}
		return false;
	}

	BitSet::Element* BitSet::Data() const
	{
		return m_pMemory;
	}

	size_t BitSet::DataSize() const
	{
		return m_Capacity/8;
	}
}

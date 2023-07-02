#include "BitSet.h"

namespace Glory::Utils
{
	BitSet::BitSet(size_t capacity, bool defaultOn) : m_pMemory(new Element[capacity + (32 - capacity % 32)]), m_Capacity(capacity) {
		for (size_t i = 0; i < capacity; ++i)
		{
			m_pMemory[i] = defaultOn ? 0xFFFFFFFF : 0;
		}
	}

	BitSet::~BitSet()
	{
		delete m_pMemory;
		m_pMemory = nullptr;

		m_Capacity = 0;
	}

	void BitSet::Set(Element index)
	{
		const Element elementIndex = index / (sizeof(Element) * 8);
		const Element bitIndex = index - elementIndex * sizeof(Element) * 8;
		m_pMemory[elementIndex] |= 1 << bitIndex;
	}

	void BitSet::UnSet(Element index)
	{
		const Element elementIndex = index / (sizeof(Element) * 8);
		const uint32_t bitIndex = index - elementIndex * sizeof(Element) * 8;
		m_pMemory[elementIndex] &= ~(1 << bitIndex);
	}

	bool BitSet::IsSet(Element index)
	{
		const Element elementIndex = index / (sizeof(Element) * 8);
		const uint32_t bitIndex = index - elementIndex * sizeof(Element) * 8;
		return m_pMemory[elementIndex] & 1 << bitIndex;
	}

	void BitSet::Reserve(size_t capacity)
	{
		if (m_Capacity >= capacity) return;
		Element* pNewMemory = new Element[capacity + (32 - capacity % 32)];
		std::memcpy(pNewMemory, m_pMemory, m_Capacity);
		delete m_pMemory;
		m_pMemory = pNewMemory;
	}
}

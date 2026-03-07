#pragma once
#include "SparseTypeTraits.h"

#include <memory>
#include <unordered_map>
#include <algorithm>
#include <cassert>

namespace Glory::Utils::ECS
{
	template<typename Element, size_t pageSize>
	struct PaginatedArray
	{
	public:
		struct Page
		{
		public:
			Page() : m_Size(0) {}
			Page(Page&& other) noexcept:
				m_Elements(std::move(other.m_Elements)), m_Size(other.m_Size) {}

			Page& operator=(Page&& other) noexcept
			{
				m_Elements = std::move(other.m_Elements);
				m_Size = other.m_Size;
				return *this;
			}

			Element* operator[](size_t index)
			{
				if (index >= m_Size) return nullptr;
				return &m_Elements[index];
			}

			void Add(const Element& elem)
			{
				assert(m_Size < MAX_PAGE_SIZE);
				const uint8_t newSize = m_Size + 1;
				Element* newElements = new Element[newSize];
				std::memcpy(&newElements[0], &m_Elements[0], sizeof(Element)*m_Size);
				newElements[m_Size] = elem;
				m_Elements.reset(newElements);
				++m_Size;
			}

			void Add(Element&& elem)
			{
				assert(m_Size < MAX_PAGE_SIZE);
				const size_t newSize = m_Size + 1;
				Element* newElements = new Element[newSize];
				for (size_t i = 0; i < m_Size; ++i)
					newElements[i] = std::move(m_Elements[i]);
				newElements[m_Size] = std::move(elem);
				m_Elements.reset(newElements);
				++m_Size;
			}

			void Insert(uint8_t index, const Element& elem)
			{
				assert(index < MAX_PAGE_SIZE);
				if (index < m_Size)
				{
					m_Elements[index] = elem;
					return;
				}

				const uint8_t newSize = index + 1;
				Element* newElements = new Element[newSize];
				std::memcpy(&newElements[0], &m_Elements[0], sizeof(Element)*m_Size);
				std::memset(&newElements[m_Size], Element(0), sizeof(Element)*(newSize - m_Size));
				newElements[index] = elem;
				m_Elements.reset(newElements);
				m_Size = newSize;
			}

			void Insert(uint8_t index, Element&& elem)
			{
				assert(index < MAX_PAGE_SIZE);
				if (index < m_Size)
				{
					m_Elements[index] = std::move(elem);
					return;
				}

				const size_t newSize = index + 1;
				Element* newElements = new Element[newSize];
				for (size_t i = 0; i < m_Size; ++i)
					newElements[i] = std::move(m_Elements[i]);
				std::memset(&newElements[m_Size], Element(0), sizeof(Element)*(newSize - m_Size));
				newElements[index] = std::move(elem);
				m_Elements.reset(newElements);
				m_Size = newSize;
			}

			static constexpr uint8_t MAX_PAGE_SIZE = pageSize;

		private:
			std::unique_ptr<Element[]> m_Elements;
			uint8_t m_Size;
		};

		PaginatedArray(uint32_t pageCount=1000u/Page::MAX_PAGE_SIZE) :
			m_Pages{ new Page[pageCount] }, m_PageCount{ pageCount }
		{
		}

		Element* operator[](size_t index)
		{
			const uint32_t pageIndex = uint32_t(index/Page::MAX_PAGE_SIZE);
			const uint8_t elementIndex = uint8_t(index%Page::MAX_PAGE_SIZE);
			if (pageIndex >= m_PageCount)
				ReservePages(std::max(pageIndex*2, 1u));
			return m_Pages[pageIndex][elementIndex];
		}

		const Element* operator[](size_t index) const
		{
			const uint32_t pageIndex = uint32_t(index / Page::MAX_PAGE_SIZE);
			const uint8_t elementIndex = uint8_t(index % Page::MAX_PAGE_SIZE);
			assert(pageIndex < m_PageCount);
			return m_Pages[pageIndex][elementIndex];
		}

		void ReservePages(uint32_t pageCount)
		{
			if (pageCount <= m_PageCount) return;
			Page* newPages = new Page[pageCount];
			for (size_t i = 0; i < m_PageCount; ++i)
				newPages[i] = std::move(m_Pages[i]);
			m_Pages.reset(newPages);
			m_PageCount = pageCount;
		}

		void Insert(size_t index, const Element& elem)
		{
			const uint32_t pageIndex = uint32_t(index/Page::MAX_PAGE_SIZE);
			const uint8_t elementIndex = uint8_t(index%Page::MAX_PAGE_SIZE);
			if (pageIndex >= m_PageCount)
			{
				ReservePages(std::max(pageIndex*2, 1u));
				m_Pages[pageIndex].Insert(elementIndex, elem);
			}
			m_Pages[pageIndex].Insert(elementIndex, elem);
		}

		void Insert(size_t index, Element&& elem)
		{
			const uint32_t pageIndex = uint32_t(index/Page::MAX_PAGE_SIZE);
			const uint8_t elementIndex = uint8_t(index%Page::MAX_PAGE_SIZE);
			if (pageIndex >= m_PageCount)
			{
				ReservePages(std::max(pageIndex*2, 1u));
				m_Pages[pageIndex].Add(std::move(elem));
				return;
			}
			m_Pages[pageIndex].Insert(index, std::move(elem));
		}

		const uint8_t MaxPageSize = pageSize;

	private:
		std::unique_ptr<Page[]> m_Pages;
		uint32_t m_PageCount;
	};

	template<typename Sparse, typename Dense, size_t pageSize=4>
	requires SparseCompatible<Sparse, Dense>
	class SparseSet
	{
	public:
		SparseSet(Sparse sparseCapacity=1000, size_t denseCapacity=32):
			m_SparseCapacity(sparseCapacity),
			m_DenseCapacity(denseCapacity),
			m_DenseSize(0),
			m_Sparse(uint32_t(sparseCapacity/pageSize)),
			m_Dense(new Dense[denseCapacity]),
			m_DenseIDs(new Sparse[denseCapacity])
		{
		}

		void ReserveSparse(Sparse newCapacity)
		{
			if (newCapacity <= m_SparseCapacity) return;
			m_Sparse.ReservePages(uint32_t(newCapacity/PaginatedArray<size_t, pageSize>::Page::MAX_PAGE_SIZE));
			m_SparseCapacity = newCapacity;
			OnReserveSparse();
		}

		void ReserveDense(size_t newCapacity)
		{
			if (newCapacity <= m_DenseCapacity) return;
			Dense* newDense = new Dense[newCapacity];
			for (size_t i = 0; i < m_DenseSize; ++i)
				newDense[i] = std::move(m_Dense[i]);
			m_Dense.reset(newDense);
			Sparse* newDenseIDs = new Sparse[newCapacity];
			std::memcpy(&newDenseIDs[0], &m_DenseIDs[0], sizeof(Sparse)*m_DenseSize);
			m_DenseIDs.reset(newDenseIDs);

			m_DenseCapacity = newCapacity;
			OnReserveDense();
		}

		Dense& Add(Sparse sparseID, Dense&& dense)
		{
			if (sparseID >= m_SparseCapacity)
				ReserveSparse(sparseID + std::max(size_t(sparseID/2), 1ull));

			if (m_DenseSize == m_DenseCapacity)
				ReserveDense(m_DenseCapacity + std::max(size_t(m_DenseCapacity/2), 0ull));

			const size_t denseIndex = m_DenseSize;
			++m_DenseSize;

			m_Sparse.Insert(size_t(sparseID), denseIndex);
			m_Dense[denseIndex] = std::move(dense);
			m_DenseIDs[denseIndex] = sparseID;

			OnAdd(denseIndex, sparseID, m_Dense[denseIndex]);
			return m_Dense[denseIndex];
		}

		void Swap(size_t index1, size_t index2)
		{
			Sparse& id1 = m_DenseIDs[index1];
			Sparse& id2 = m_DenseIDs[index2];
			*m_Sparse[id1] = index2;
			*m_Sparse[id2] = index1;
			std::swap(m_Dense[index1], m_Dense[index2]);
			std::swap(m_DenseIDs[index1], m_DenseIDs[index2]);
			OnSwap(index1, index2);
		}

		void Remove(Sparse sparseID)
		{
			if (sparseID >= m_SparseCapacity)
			{
				ReserveSparse(sparseID + size_t(sparseID/2));
				return;
			}

			size_t* index = m_Sparse[sparseID];
			if (!index || *index) return;
			const size_t oldSize = m_DenseSize;
			--m_DenseSize;
			if (m_DenseSize == 0)
			{
				*index = 0;
				m_DenseIDs[0] = Sparse(0);
				OnRemove(sparseID, *index);
				return;
			}
			Swap(*index, m_DenseSize);
			OnRemove(sparseID, *index);
		}

		Dense& Get(Sparse sparse)
		{
			auto index = m_Sparse[sparse];
			assert(index);
			return m_Dense[*index];
		}

		const Dense& Get(Sparse sparse) const
		{
			auto index = m_Sparse[sparse];
			assert(index);
			return m_Dense[*index];
		}

		inline Dense& GetAt(size_t index)
		{
			return m_Dense[index];
		}

		inline const Dense& GetAt(size_t index) const
		{
			return m_Dense[index];
		}

		inline size_t Size() const
		{
			return m_DenseSize;
		}

		inline size_t Index(Sparse sparse) const
		{
			const size_t* index = m_Sparse[sparse];

			return index ? *index : InvalidIndex;
		}

		inline Sparse DenseID(size_t index) const
		{
			return m_DenseIDs[index];
		}

		inline Sparse SparseCapacity() const
		{
			return m_SparseCapacity;
		}

		inline size_t DenseCapacity() const
		{
			return m_DenseCapacity;
		}

		struct Iterator
		{
			Iterator(Dense* pDense, size_t index) :
				m_DenseArray(pDense) {
			}

			Dense& operator*() { return m_DenseArray[m_Index]; }
			const Dense& operator*() const { return m_DenseArray[m_Index]; }

			// Prefix increment
			Iterator& operator++() { ++m_Index; return *this; }

			// Postfix increment
			Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

			Iterator operator+(int num) { m_Index += num; return *this; }

			friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_DenseArray == b.m_DenseArray && a.m_Index == b.m_Index; };
			friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_DenseArray != b.m_DenseArray || a.m_Index != b.m_Index; };

		private:
			Dense* m_DenseArray;
			size_t m_Index;
		};

		inline Iterator Begin() const
		{
			return Iterator(&m_Dense[0], 0);
		}

		inline Iterator End() const
		{
			return Iterator(&m_Dense[0], m_DenseSize);
		}

		static constexpr size_t InvalidIndex = UINT64_MAX;

	protected:
		virtual void OnAdd(size_t, Sparse, Dense&) {};
		virtual void OnRemove(Sparse, size_t) {};
		virtual void OnReserveDense() {};
		virtual void OnReserveSparse() {};
		virtual void OnSwap(size_t index1, size_t index2) {};

	private:
		PaginatedArray<size_t, pageSize> m_Sparse;
		std::unique_ptr<Dense[]> m_Dense;
		std::unique_ptr<Sparse[]> m_DenseIDs;
		Sparse m_SparseCapacity;
		size_t m_DenseCapacity;
		size_t m_DenseSize;
	};
}

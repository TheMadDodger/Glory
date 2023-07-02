#pragma once
#include <cstring>
#include <limits>
#include <exception>

namespace Glory::Utils
{
	template<typename TElem, typename TIndex>
	class BinaryTree
	{
	private:
		struct TreeNode
		{
			static constexpr size_t InvalidIndex = std::numeric_limits<size_t>::max();

			TElem Data;
			TIndex Index;

			size_t SmallerChildIndex = InvalidIndex;
			size_t LargerChildIndex = InvalidIndex;

			bool HasSmaller() {
				return SmallerChildIndex != InvalidIndex;
			}

			bool HasLarger() {
				return LargerChildIndex != InvalidIndex;
			}
		}*m_pMemory = nullptr;

	public:
		BinaryTree(size_t capacity) : m_pMemory{ new TreeNode[capacity] }, m_Capacity{capacity}, m_Size(0)
		{}

		~BinaryTree()
		{
			delete m_pMemory;
			m_pMemory = nullptr;
		}

		size_t Size() { return m_Size; }
		size_t Capacity() { return m_Capacity; }

		TreeNode& Insert(const TElem& element, const TIndex& index)
		{
			size_t elementIndex;
			TreeNode& newNode = GetNewNode(elementIndex);
			newNode.Data = element;
			newNode.Index = index;

			if (m_Size == 1) {
				return newNode;
			}

			TreeNode& firstNode = m_pMemory[0];
			InsertInto(elementIndex, newNode, firstNode);
			return newNode;
		}

		TElem& Get(TIndex index)
		{
			return BinarySearch(index).Data;
		}

		bool Has(TIndex index)
		{
			return BinaryFind(index);
		}

		struct Iterator
		{
			Iterator(TreeNode* ptr): m_Ptr(ptr) {}

			TElem& operator*() const { return m_Ptr->Data; }
			TElem* operator->() { return &m_Ptr->Data; }

			// Prefix increment
			Iterator& operator++() { m_Ptr++; return *this; }

			// Postfix increment
			Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

			friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_Ptr == b.m_Ptr; };
			friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_Ptr != b.m_Ptr; };

		private:
			TreeNode* m_Ptr;
		};

		Iterator begin()
		{
			return Iterator(m_pMemory);
		}

		Iterator end()
		{
			return Iterator(&m_pMemory[m_Size]);
		}

	private:
		void InsertInto(size_t index, TreeNode& newNode, TreeNode& node)
		{
			if (newNode.Index > node.Index)
			{
				if (node.HasLarger())
				{
					TreeNode& largerNode = m_pMemory[node.LargerChildIndex];
					InsertInto(index, newNode, largerNode);
					return;
				}
				node.LargerChildIndex = index;
			}
			else if (newNode.Index < node.Index)
			{
				if (node.HasSmaller())
				{
					TreeNode& smallerNode = m_pMemory[node.SmallerChildIndex];
					InsertInto(index, newNode, smallerNode);
					return;
				}
				node.SmallerChildIndex = index;
			}
		}

		TreeNode& GetNewNode(size_t& index)
		{
			index = m_Size;
			EnsureCapacity(m_Size + 1);
			++m_Size;
			return m_pMemory[m_Size-1];
		}

		void EnsureCapacity(size_t capacity)
		{
			if (m_Capacity >= capacity) return;
			m_Capacity *= 2;
			TreeNode* pNewMemory = new TreeNode[m_Capacity];
			std::memcpy(pNewMemory, m_pMemory, m_Size * sizeof(TreeNode));
			delete m_pMemory;
			m_pMemory = pNewMemory;
		}

		TreeNode& BinarySearch(TIndex index)
		{
			TreeNode* pNextNode = m_pMemory;

			while (pNextNode)
			{
				if (index < pNextNode->Index)
				{
					if (!pNextNode->HasSmaller())
						throw new std::exception("BinaryTree:Search> Index not found!");
					pNextNode = &m_pMemory[pNextNode->SmallerChildIndex];
				}
				else if (index > pNextNode->Index)
				{
					if (!pNextNode->HasLarger())
						throw new std::exception("BinaryTree:Search> Index not found!");
					pNextNode = &m_pMemory[pNextNode->LargerChildIndex];
				}
				else
				{
					return *pNextNode;
				}
			}
			
			throw new std::exception("BinaryTree:Search> Unreachable code reached!");
		}

		bool BinaryFind(TIndex index)
		{
			TreeNode* pNextNode = m_pMemory;

			while (pNextNode)
			{
				if (pNextNode->Index < index)
				{
					if (!pNextNode->HasSmaller())
						return false;
					pNextNode = &m_pMemory[pNextNode->SmallerChildIndex];
				}
				else if (pNextNode->Index > index)
				{
					if (!pNextNode->HasLarger())
						return false;
					pNextNode = &m_pMemory[pNextNode->LargerChildIndex];
				}
				else
				{
					return true;
				}
			}

			throw new std::exception("BinaryTree:Search> Unreachable code reached!");
		}

	private:
		size_t m_Capacity;
		size_t m_Size;
	};
}

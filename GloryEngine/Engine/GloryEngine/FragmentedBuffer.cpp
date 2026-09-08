#include "FragmentedBuffer.h"

#include <GloryAssert.h>

#include <optional>
#include <algorithm>

namespace Glory
{
	FragmentedBuffer::FragmentedBuffer()
	{
	}

	FragmentedBuffer::FragmentedBuffer(size_t size, BufferHandle bufferHandle):
		m_Size(size), m_BufferHandle(bufferHandle)
	{
		if (m_Size == 0)
			return;

		m_Fragments.emplace_back(0ull, size);
	}

	FragmentedBuffer::FragmentedBuffer(FragmentedBuffer&& other) noexcept:
		m_Fragments(std::move(other.m_Fragments)), m_Size(other.m_Size), m_BufferHandle(other.m_BufferHandle)
	{
		other.m_Size = 0ull;
		other.m_BufferHandle = nullptr;
	}

	FragmentedBuffer::FragmentedBuffer(size_t size, std::vector<std::pair<size_t, size_t>>&& fragments, BufferHandle bufferHandle):
		m_Fragments(std::move(fragments)), m_Size(size), m_BufferHandle(bufferHandle)
	{
		std::sort(m_Fragments.begin(), m_Fragments.end(), [](const auto& a, const auto& b) {
			return a.first < b.first;
		});
	}

	FragmentedBuffer::~FragmentedBuffer()
	{
	}

	FragmentedBuffer& FragmentedBuffer::operator=(FragmentedBuffer&& other) noexcept
	{
		m_Fragments = std::move(other.m_Fragments);
		m_Size = other.m_Size;
		m_BufferHandle = other.m_BufferHandle;

		other.m_Size = 0ull;
		other.m_BufferHandle = nullptr;
		return *this;
	}

	bool FragmentedBuffer::FindFreeFragment(size_t requiredSize, size_t& fragmentIndex)
	{
		bool found = false;
		/* Find the smallest fragment that can fit this data */
		for (size_t i = 0; i < m_Fragments.size(); ++i)
		{
			if (requiredSize > m_Fragments[i].second)
				continue;

			if (found && m_Fragments[fragmentIndex].second < m_Fragments[i].second)
				continue;

			fragmentIndex = i;
			found = true;
		}

		return found;
	}

	FragmentedBuffer::operator bool() const
	{
		return m_Size > 0 && m_BufferHandle;
	}

	FragmentedBuffer::operator BufferHandle() const
	{
		return m_BufferHandle;
	}

	BufferHandle FragmentedBuffer::Buffer() const
	{
		return m_BufferHandle;
	}

	size_t FragmentedBuffer::PushData(void* data, size_t size, size_t fragmentIndex, GraphicsDevice* pDevice)
	{
		GLORY_ASSERT(fragmentIndex < m_Fragments.size(), "Fragment index out of range!");
		GLORY_ASSERT(size <= m_Fragments[fragmentIndex].second, "Fragment is too small for data!");

		const size_t startIndex = m_Fragments[fragmentIndex].first;

		/* Resize the fragment */
		m_Fragments[fragmentIndex].second -= size;
		m_Fragments[fragmentIndex].first += size;

		if (m_Fragments[fragmentIndex].second <= 0ull)
			m_Fragments.erase(m_Fragments.begin() + fragmentIndex);

		if (!m_BufferHandle)
			return startIndex;

		/* Assign data to buffer at index */

		return startIndex;
	}

	void FragmentedBuffer::FreeData(size_t start, size_t size, GraphicsDevice* pDevice)
	{
		/* Find fragments adjacent to data */
		std::optional<size_t> leftAdjacentFragment;
		std::optional<size_t> rightAdjacentFragment;

		const size_t end = start + size;

		for (size_t i = 0; i < m_Fragments.size(); ++i)
		{
			if (leftAdjacentFragment && rightAdjacentFragment)
				break;

			const size_t fragmentEnd = m_Fragments[i].first + m_Fragments[i].second;

			GLORY_ASSERT(start < m_Fragments[i].first || start >= fragmentEnd,
				"Invalid data range!");
			GLORY_ASSERT(end <= m_Fragments[i].first || end > fragmentEnd,
				"Invalid data range!");

			if (end == m_Fragments[i].first)
				rightAdjacentFragment = i;

			if (start == fragmentEnd)
				leftAdjacentFragment = i;
		}

		/* If we have both a left and right we have to combine the 2 and discard the data in the middle */
		if (leftAdjacentFragment && rightAdjacentFragment)
		{
			const size_t leftIndex = *leftAdjacentFragment;
			const size_t rightIndex = *rightAdjacentFragment;
			const size_t totalFragmentSize = m_Fragments[leftIndex].second + size + m_Fragments[rightIndex].second;

			m_Fragments[leftIndex].second = totalFragmentSize;
			m_Fragments.erase(m_Fragments.begin() + rightIndex);
			return;
		}

		/* If only left fragment we only have to resize the fragment */
		if (leftAdjacentFragment)
		{
			const size_t leftIndex = *leftAdjacentFragment;
			const size_t totalFragmentSize = m_Fragments[leftIndex].second + size;
			m_Fragments[leftIndex].second = totalFragmentSize;
			return;
		}

		/* If only right fragment we have to resize and move the start */
		if (rightAdjacentFragment)
		{
			const size_t rightIndex = *rightAdjacentFragment;
			const size_t totalFragmentSize = m_Fragments[rightIndex].second + size;
			m_Fragments[rightIndex].first = start;
			m_Fragments[rightIndex].second = totalFragmentSize;
			return;
		}

		/* No adjacent fragments, insert new fragment */
		const size_t index = m_Fragments.size();
		m_Fragments.emplace_back(start, size);

		/* Sort the new fragment into place */
		for (size_t i = index; i > 0; --i)
		{
			if (m_Fragments[i].first > m_Fragments[i - 1].first)
				break;

			std::swap(m_Fragments[i], m_Fragments[i - 1]);
		}
	}

	size_t FragmentedBuffer::Size() const
	{
		return m_Size;
	}

	size_t FragmentedBuffer::FragmentCount() const
	{
		return m_Fragments.size();
	}

	const std::pair<size_t, size_t>& FragmentedBuffer::Fragment(size_t index) const
	{
		GLORY_ASSERT(index < m_Fragments.size(), "Fragment index out of bounds.");
		return m_Fragments[index];
	}
}

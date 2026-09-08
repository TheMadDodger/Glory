#pragma once
#include "engine_visibility.h"

#include "GraphicsHandles.h"

#include <vector>

namespace Glory
{
	class GraphicsDevice;

	/** @brief Fragmented buffer */
	class FragmentedBuffer
	{
	public:
		/** @brief Constructor */
		GLORY_ENGINE_API FragmentedBuffer();
		/** @overload
		 * @param size The size of the buffer
		 * @param bufferHandle The buffer handle
		 */
		GLORY_ENGINE_API FragmentedBuffer(size_t size, BufferHandle bufferHandle);
		/** @overload move constructor */
		GLORY_ENGINE_API FragmentedBuffer(FragmentedBuffer&& other) noexcept;
		/** @overload
		 * @param size The size of the buffer
		 * @param fragments Buffer fragments (start, size)
		 * @param bufferHandle Handle to the buffer on the GPU
		 */
		GLORY_ENGINE_API FragmentedBuffer(size_t size, std::vector<std::pair<size_t, size_t>>&& fragments,
			BufferHandle bufferHandle);
		/** @overload Deleted copy constructor */
		GLORY_ENGINE_API FragmentedBuffer(const FragmentedBuffer&) = delete;
		/** @brief Destructor */
		GLORY_ENGINE_API ~FragmentedBuffer();

		/** @brief Move assignment operator */
		GLORY_ENGINE_API FragmentedBuffer& operator=(FragmentedBuffer&& other) noexcept;
		/** @brief Deleted copy assignment operator */
		GLORY_ENGINE_API FragmentedBuffer& operator=(const FragmentedBuffer&) = delete;

		/** @brief Find a free fragment in the buffer
		 * @param requiredSize Required size to be free
		 * @param fragmentIndex The found fragment index
		 * @returns @cpp true @ce if found @cpp false @ce otherwise
		 */
		GLORY_ENGINE_API bool FindFreeFragment(size_t requiredSize, size_t& fragmentIndex);

		/** @brief Boolean operator */
		GLORY_ENGINE_API explicit operator bool() const;
		/** @brief @ref BufferHandle operator */
		GLORY_ENGINE_API explicit operator BufferHandle() const;
		/** @brief Handle to the GPU buffer */
		GLORY_ENGINE_API BufferHandle Buffer() const;

		/** @brief Push data to the buffer on a chosen fragment
		 * @param data The data to push
		 * @param size Size of the data to push
		 * @param fragmentIndex Index of the fragment to use
		 * @param pDevice Device that owns the buffer
		 * @returns Start byte index of the newly pushed data
		 */
		GLORY_ENGINE_API size_t PushData(void* data, size_t size, size_t fragmentIndex, GraphicsDevice* pDevice);
		/** @brief Free data and update fragments
		 * @param start The start byte index
		 * @param size Size of the data to free
		 * @param pDevice Device that owns the buffer
		 */
		GLORY_ENGINE_API void FreeData(size_t start, size_t size, GraphicsDevice* pDevice);

		/** @brief Maximum size of the buffer */
		GLORY_ENGINE_API size_t Size() const;
		/** @brief Current fragment count in the buffer */
		GLORY_ENGINE_API size_t FragmentCount() const;
		/** @brief Get a fragment by index */
		GLORY_ENGINE_API const std::pair<size_t, size_t>& Fragment(size_t index) const;

	private:
		/* pair<start, size> */
		std::vector<std::pair<size_t, size_t>> m_Fragments;

		size_t m_Size = 0;
		BufferHandle m_BufferHandle = nullptr;
	};
}

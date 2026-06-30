#pragma once
#include <engine_visibility.h>

#include <atomic>

#include <UUID.h>

namespace Glory
{
	class Resource;
	class Resources;

	/** @brief Resource loader base class. */
	class ResourceLoader
	{
	public:
		/** @brief Constructor */
		GLORY_ENGINE_API ResourceLoader();
		/** @brief Destructor */
		GLORY_ENGINE_API virtual ~ResourceLoader();

		/** @brief Iterate over all queued for loading and unloading resources and
		 * call the loading and unloading implementations. */
		GLORY_ENGINE_API void Update();
		/** @brief Set @ref Resources instance. */
		GLORY_ENGINE_API void SetResources(Resources* pResources);
		GLORY_ENGINE_API Resource* LoadImmediately(UUID id);

		/** @brief Stall the current thread until the next update call */
		GLORY_ENGINE_API void WaitForNextUpdate();

		virtual bool IsBusy() const = 0;

	protected:
		/** @brief Override to add loading implementation. */
		virtual void QueueLoad(UUID id, bool immediate=false) = 0;
		/** @brief Override to add unloading implementation. */
		virtual void QueueUnload(UUID id) = 0;
		/** @brief Override to do custom processing. */
		virtual void OnUpdate() = 0;

	protected:
		Resources* m_pResources = nullptr;
		std::atomic_uint64_t m_UpdateCounter = 0;
	};
}

#pragma once
#include "engine_visibility.h"

#include "ResourceManager.h"

#include <GloryAssert.h>
#include <Reflection.h>

#include <array>
#include <set>
#include <functional>
#include <mutex>

namespace Glory
{
	class Debug;
	class AssetDatabase;
	class ThreadManager;

	/** @brief Global resource manager */
	class Resources
	{
	public:
		/** @brief Constructor
		 * @param pDatabase Asset database
		 * @param pResourceTypes Resource types manager
		 * @param pDebug Debug logging instance
		 */
		Resources(AssetDatabase* pDatabase, ResourceTypes* pResourceTypes, Debug* pDebug, ThreadManager* pThreads);
		/** @brief Destructor */
		~Resources() = default;

	public:
		/** @brief Register a resource type and create a manager for it
		 * @param handler Callback that must be called when a resource of this type is created by a resource factory
		 */
		template<typename R>
		requires ResourceCompatible<R>
		inline void RegisterResource(std::function<void(R*)>&& handler=NULL)
		{
			m_pResourceTypes->RegisterResource<R>("", std::move(handler));
			const size_t managerIndex = m_Managers.size();
			ResourceManager<R>* pManager = new ResourceManager<R>(this);
			m_HashToManagerIndex.emplace(pManager->TypeHash, managerIndex);
			m_Managers.emplace_back(pManager);
			Reflect::RegisterResourceType<R>();
		}

		/** @brief Add a resource to a compatible manager
		 * @param pResource Resource to add
		 * @returns @cpp true @ce if the resource was succesfully moved to a compatible manager
		 *
		 * Note that the original resource will be deleted as its data will be moved to the manager,
		 * and the pointer will be set to the new resource.
		 * If no compatible manager is found, the resource is not deleted.
		 */
		template<typename R>
		requires ResourceCompatible<R>
		inline bool AddResource(R** pResource)
		{
			return AddResource((Resource**)(pResource));
		}

		/** @brief Find a resource by ID and return it.
		 * @param id ID of the resource to find.
		 * @returns Pointer to the resource if found, otherwise nullptr.
		 *
		 * Will throw if the asset is loaded under a different type.
		 */
		template<typename R>
		requires ResourceCompatible<R>
		inline R* GetResource(UUID id)
		{
			auto iter = m_ResourceIDToManagerIndex.find(id);
			if (iter == m_ResourceIDToManagerIndex.end()) return nullptr;
			const uint32_t hash = ResourceTypes::GetHash<R>();
			auto& managerInterface = m_Managers.at(iter->second);
			const uint32_t actualType = managerInterface->Type();
			GLORY_ASSERT(hash == actualType, "Resource type hash mismatch.");
			ResourceManager<R>* manager = static_cast<ResourceManager<R>*>(managerInterface.get());
			return manager->GetDirect(id);
		}

		/** @overload */
		GLORY_ENGINE_API bool AddResource(Resource** pResource);
		/** @overload
		 *
		 * Uses a cache to speed up in which manager to find the resource.
		 */
		GLORY_ENGINE_API Resource* GetResource(UUID id);
		/** @brief Get the @ref ResourceTypes instance attached to this resource manager */
		GLORY_ENGINE_API ResourceTypes* GetResourceTypes();

		/** @brief Increment the reference counter for a resource by ID
		 * @param id ID of the resource
		 *
		 * If the counter was 0, then this will queue the resource for loading.
		 */
		GLORY_ENGINE_API void AddReference(UUID id);
		/** @brief Decrement the reference counter for a resource by ID
		 * @param id ID of the resource
		 *
		 * If the counter becomes 0 after decrementing, then this will queue the resource for unloading.
		 */
		GLORY_ENGINE_API void RemoveReference(UUID id);

		/** @brief Get the current reference count of a resource. */
		GLORY_ENGINE_API uint64_t ReferenceCount(UUID resourceID);

		/** @brief Run a callback on each queued for loading resource ID, then clear the queue.
		 * @param callback Function to call on each resource ID.
		 */
		GLORY_ENGINE_API void HandleToLoad(std::function<void(UUID)> callback);
		/** @brief Run a callback on each queued for immediate loading resource ID, then clear the queue.
		 * @param callback Function to call on each resource ID.
		 */
		GLORY_ENGINE_API void HandleToLoadImmediately(std::function<void(UUID)> callback);
		/** @brief Run a callback on each queued for unloading resource ID, then clear the queue.
		 * @param callback Function to call on each resource ID.
		 */
		GLORY_ENGINE_API void HandleToUnload(std::function<void(UUID)> callback);

		/** @brief Remove a resource from its resource manager.
		 * @param id ID of the resource to remove.
		 *
		 * Just like @ref GetResource() this uses a cache to speed up finding the manager.
		 * It should be noted that a removed resource is simply moved to the end of the array,
		 * once the manager grows to that size it is replaced by the new resource.
		 * This means unloading is fast and memory efficient.
		 */
		GLORY_ENGINE_API void UnloadResource(UUID id);

		/** @brief Set whether reference counting should be blocked. */
		GLORY_ENGINE_API void SetAllowReferenceCounting(bool allowed);

	private:
		std::vector<std::unique_ptr<IResourceManager>> m_Managers;
		std::map<uint32_t, size_t> m_HashToManagerIndex;
		std::map<UUID, size_t> m_ResourceIDToManagerIndex;

		std::mutex m_ReferenceCounterLock;
		std::map<UUID, std::atomic_uint64_t> m_ReferenceCounter;

		uint32_t m_CurrentLoadImmediatelyReadIndex = 0;
		std::atomic_uint64_t m_LoadImmediatelyWriteIndex = 0ull;
		std::atomic_uint64_t m_CurrentLoadImmediatelyCount = 0ull;
		static constexpr size_t LoadImmediatelyRingBufferSize = 1024;
		std::array<UUID, LoadImmediatelyRingBufferSize> m_ToLoadImmediately;

		std::set<UUID> m_ToLoadResources;
		std::set<UUID> m_ToUnloadResources;

		AssetDatabase* m_pDatabase;
		ResourceTypes* m_pResourceTypes;
		Debug* m_pDebug;
		ThreadManager* m_pThreads;

		bool m_ReferenceCountingAllowed = true;
		bool m_AlreadyLoading = true;
	};
}

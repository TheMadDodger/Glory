#include "Resources.h"
#include "Debug.h"
#include "ResourceReferencing.h"
#include "AssetDatabase.h"

#include <Hash.h>
#include <GloryAssert.h>

#include <ThreadManager.h>

namespace Glory
{
	Resources::Resources(AssetDatabase* pDatabase, ResourceTypes* pResourceTypes, Debug* pDebug, ThreadManager* pThreads):
		m_pDatabase(pDatabase), m_pResourceTypes(pResourceTypes), m_pDebug(pDebug), m_pThreads(pThreads), m_ReferenceCounter()
	{
		SetResourcesInstance(this);
	}

	bool Resources::AddResource(Resource** pResource)
	{
		std::type_index type = typeid(Object);
		for (size_t i = 0; i < (*pResource)->TypeCount(); ++i)
		{
			(*pResource)->GetType(i, type);
			const uint32_t typeHash = Hashing::Hash(type.name());
			auto iter = m_HashToManagerIndex.find(typeHash);
			if (iter == m_HashToManagerIndex.end()) continue;
			Resource* pAddedResource = m_Managers.at(iter->second)->Add((*pResource));
			m_ResourceIDToManagerIndex.emplace(pAddedResource->GetUUID(), iter->second);
			*pResource = pAddedResource;
			return true;
		}

		(*pResource)->GetType(0, type);
		m_pDebug->LogError(std::format("Resources::AddResource: Could not add resource, missing resource manager for type {}", type.name()));
		return false;
	}

	Resource* Resources::GetResource(UUID id)
	{
		auto iter = m_ResourceIDToManagerIndex.find(id);
		if (iter == m_ResourceIDToManagerIndex.end()) return nullptr;
		return m_Managers.at(iter->second)->Get(id);
	}

	ResourceTypes* Resources::GetResourceTypes()
	{
		return m_pResourceTypes;
	}

	void Resources::AddReference(UUID id)
	{
		if (!m_ReferenceCountingAllowed) return;

		std::unique_lock lock{ m_ReferenceCounterLock };
		std::atomic_uint64_t& counter = m_ReferenceCounter[id];
		lock.unlock();

		const uint64_t count = counter.fetch_add(1);
		if (count > 0) return;
		if (!m_pDatabase->AssetExists(id)) return;
		/* Check if current thread id is not the default thread and queue immedeitaly if it is not */
		if (m_AlreadyLoading || std::this_thread::get_id() != m_pThreads->DefaultThreadID())
		{
			const uint64_t nextIndex = m_LoadImmediatelyWriteIndex.fetch_add(1) % LoadImmediatelyRingBufferSize;
			m_ToLoadImmediately[nextIndex] = id;
			m_CurrentLoadImmediatelyCount.fetch_add(1);
			return;
		}

		m_ToUnloadResources.erase(id);
		m_ToLoadResources.insert(id);
	}

	void Resources::RemoveReference(UUID id)
	{
		if (!m_ReferenceCountingAllowed) return;
		std::unique_lock lock{ m_ReferenceCounterLock };
		std::atomic_uint64_t& counter = m_ReferenceCounter[id];
		lock.unlock();

		uint64_t count = counter.load();
		GLORY_ASSERT(count > 0, "Incorrect reference count!");
		count = counter.fetch_sub(1);
		if (count != 1) return;
		if (!m_pDatabase->AssetExists(id)) return;
		m_ToUnloadResources.insert(id);
	}

	uint64_t Resources::ReferenceCount(UUID id)
	{
		std::unique_lock lock{ m_ReferenceCounterLock };
		auto iter = m_ReferenceCounter.find(id);
		return iter != m_ReferenceCounter.end() ? iter->second.load() : 0ull;
	}

	void Resources::HandleToLoad(std::function<void(UUID)> callback)
	{
		m_AlreadyLoading = true;
		for (auto iter = m_ToLoadResources.begin(); iter != m_ToLoadResources.end(); ++iter)
			callback(*iter);
		m_ToLoadResources.clear();
		m_AlreadyLoading = false;
	}

	void Resources::HandleToLoadImmediately(std::function<void(UUID)> callback)
	{
		const uint64_t size = m_CurrentLoadImmediatelyCount.load();
		if (size == 0) return;

		m_AlreadyLoading = true;
		m_CurrentLoadImmediatelyCount.fetch_sub(size);
		for (size_t i = 0; i < size; ++i)
		{
			UUID& id = m_ToLoadImmediately[m_CurrentLoadImmediatelyReadIndex];
			++m_CurrentLoadImmediatelyReadIndex;
			m_CurrentLoadImmediatelyReadIndex = m_CurrentLoadImmediatelyReadIndex % LoadImmediatelyRingBufferSize;
			if (!id) continue;
			callback(id);
			id = 0ull;
		}
		m_AlreadyLoading = false;
	}

	void Resources::HandleToUnload(std::function<void(UUID)> callback)
	{
		/* We need to make a copy since some resources might also
		 * cause an unload of other resources when they are destroyed */
		const std::set<UUID> copy = m_ToUnloadResources;
		m_ToUnloadResources.clear();

		for (auto iter = copy.begin(); iter != copy.end(); ++iter)
			callback(*iter);
	}

	void Resources::UnloadResource(UUID id)
	{
		auto iter = m_ResourceIDToManagerIndex.find(id);
		if (iter == m_ResourceIDToManagerIndex.end()) return;
		m_Managers.at(iter->second)->Remove(id);
	}

	void Resources::SetAllowReferenceCounting(bool allowed)
	{
		m_ReferenceCountingAllowed = allowed;
	}
}

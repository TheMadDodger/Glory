#include "EntityRegistry.h"

#include <BinaryStream.h>

#include <cassert>

namespace Glory::Utils::ECS
{
	EntityRegistry::EntityRegistry(void* userData, size_t reserveComponentManagers, size_t reserveEntities):
		m_ComponentManagers(), m_ComponentOrderDirty(reserveComponentManagers),
		m_NextEntityID(1ull), m_AliveCount(0), m_EnabledCalls(32, true), m_pUserData(userData)
	{
		m_ComponentManagers.reserve(reserveComponentManagers);
		m_EntityTrees.resize(reserveEntities);
		m_Parents.resize(reserveEntities);
		m_HasComponent.resize(reserveEntities);
		m_EntityComponentOrder.resize(reserveEntities);
	}

	EntityRegistry::EntityRegistry(EntityRegistry&& other) noexcept:
		m_ComponentManagers(std::move(other.m_ComponentManagers)),
		m_HashToComponentManagerIndex(std::move(other.m_HashToComponentManagerIndex)),
		m_ComponentOrderDirty(std::move(other.m_ComponentOrderDirty)),
		m_EntityAlive(std::move(other.m_EntityAlive)),
		m_EntityActiveSelf(std::move(other.m_EntityActiveSelf)),
		m_EntityActiveHierarchy(std::move(other.m_EntityActiveHierarchy)),
		m_EntityDirty(std::move(other.m_EntityDirty)),
		m_EntityTrees(std::move(other.m_EntityTrees)),
		m_Parents(std::move(other.m_Parents)),
		m_HasComponent(std::move(other.m_HasComponent)),
		m_EntityComponentOrder(std::move(other.m_EntityComponentOrder)),
		m_NextEntityID(other.m_NextEntityID),
		m_AliveCount(other.m_AliveCount),
		m_EnabledCalls(std::move(other.m_EnabledCalls)),
		m_pUserData(other.m_pUserData),
		m_CallsEnabled(other.m_CallsEnabled)
	{
		other.m_NextEntityID = 0;
		other.m_AliveCount = 0;
		other.m_pUserData = nullptr;
		other.m_CallsEnabled = false;
	}

	EntityRegistry& EntityRegistry::operator=(EntityRegistry&& other) noexcept
	{
		m_ComponentManagers = std::move(other.m_ComponentManagers);
		m_HashToComponentManagerIndex = std::move(other.m_HashToComponentManagerIndex);
		m_ComponentOrderDirty = std::move(other.m_ComponentOrderDirty);
		m_EntityAlive = std::move(other.m_EntityAlive);
		m_EntityActiveSelf = std::move(other.m_EntityActiveSelf);
		m_EntityActiveHierarchy = std::move(other.m_EntityActiveHierarchy);
		m_EntityDirty = std::move(other.m_EntityDirty);
		m_EntityTrees = std::move(other.m_EntityTrees);
		m_Parents = std::move(other.m_Parents);
		m_HasComponent = std::move(other.m_HasComponent);
		m_EntityComponentOrder = std::move(other.m_EntityComponentOrder);
		m_NextEntityID = other.m_NextEntityID;
		m_AliveCount = other.m_AliveCount;
		m_EnabledCalls = std::move(other.m_EnabledCalls);
		m_pUserData = other.m_pUserData;
		m_CallsEnabled = other.m_CallsEnabled;

		other.m_NextEntityID = 0;
		other.m_AliveCount = 0;
		other.m_pUserData = nullptr;
		other.m_CallsEnabled = false;
		return *this;
	}

	EntityRegistry::~EntityRegistry()
	{
		m_HashToComponentManagerIndex.clear();
		m_ComponentManagers.clear();
	}

	EntityID EntityRegistry::CreateEntity()
	{
		m_EntityAlive.Reserve(m_NextEntityID);
		m_EntityActiveSelf.Reserve(m_NextEntityID);
		m_EntityActiveHierarchy.Reserve(m_NextEntityID);
		m_EntityDirty.Reserve(m_NextEntityID);

		EntityID newEntity = m_NextEntityID;
		++m_NextEntityID;

		m_EntityAlive.Set(newEntity);
		m_EntityActiveSelf.Set(newEntity);
		m_EntityActiveHierarchy.Set(newEntity);
		m_EntityDirty.Set(newEntity);

		m_EntityTrees.resize(newEntity + 1);
		m_Parents.resize(newEntity + 1);
		m_HasComponent.resize(newEntity + 1);
		m_EntityComponentOrder.resize(newEntity + 1);

		m_Parents[newEntity] = 0ull;
		m_EntityTrees[0ull].emplace_back(newEntity);
		m_HasComponent[newEntity].Reserve(m_ComponentManagers.size());
		m_HasComponent[newEntity].Clear();

		++m_AliveCount;

		return newEntity;
	}

	void* EntityRegistry::CreateComponent(EntityID entity, uint32_t componentHash, UUID componentID)
	{
		size_t index = 0;
		IComponentManager* manager = GetComponentManager(componentHash, &index);
		m_ComponentOrderDirty.Set(index);
		m_HasComponent[entity].Set(index);
		m_EntityComponentOrder[entity].emplace_back(manager->ComponentHash(), componentID);
		return manager->Add(entity);
	}

	void EntityRegistry::AddManager(IComponentManager* manager)
	{
		const uint32_t hash = manager->ComponentHash();
		const uint32_t index = uint32_t(m_ComponentManagers.size());
		m_ComponentManagers.emplace_back(manager);
		m_ComponentOrderDirty.Reserve(index + 1ull);
		m_ComponentOrderDirty.Set(index, false);
		m_HashToComponentManagerIndex.emplace(hash, index);
		manager->Initialize(index);
	}

	UUID EntityRegistry::RemoveComponent(EntityID entity, uint32_t typeHash)
	{
		size_t index = 0;
		IComponentManager* manager = GetComponentManager(typeHash, &index);
		assert(m_HasComponent[entity].IsSet(index));
		auto iter = std::find_if(m_EntityComponentOrder[entity].begin(), m_EntityComponentOrder[entity].end(),
			[manager](const std::pair<uint32_t, UUID>& pair) {
				return pair.first == manager->ComponentHash();
			});

		manager->Remove(entity);
		const UUID id = iter->second;
		m_EntityComponentOrder[entity].erase(iter);
		m_HasComponent[entity].UnSet(index);
		return id;
	}

	UUID EntityRegistry::RemoveComponentAt(EntityID entity, size_t index)
	{
		auto& pair = m_EntityComponentOrder[entity][index];
		size_t managerIndex = 0;
		IComponentManager* manager = GetComponentManager(pair.first, &managerIndex);
		manager->Remove(entity);
		const UUID id = pair.second;
		m_EntityComponentOrder[entity].erase(m_EntityComponentOrder[entity].begin() + index);
		m_HasComponent[entity].UnSet(managerIndex);
		return id;
	}


	bool EntityRegistry::HasComponent(EntityID entity, uint32_t typeHash) const
	{
		size_t index = 0;
		GetComponentManager(typeHash, &index);
		return m_HasComponent[entity].IsSet(index);
	}

	size_t EntityRegistry::ComponentManagerCount() const
	{
		return m_ComponentManagers.size();
	}

	IComponentManager* EntityRegistry::GetComponentManager(uint32_t componentHash, size_t* outIndex)
	{
		auto iter = m_HashToComponentManagerIndex.find(componentHash);
		assert(iter != m_HashToComponentManagerIndex.end());
		if (outIndex) *outIndex = iter->second;
		return m_ComponentManagers[iter->second].get();
	}

	const IComponentManager* EntityRegistry::GetComponentManager(uint32_t componentHash, size_t* outIndex) const
	{
		auto iter = m_HashToComponentManagerIndex.find(componentHash);
		assert(iter != m_HashToComponentManagerIndex.end());
		if (outIndex) *outIndex = iter->second;
		return m_ComponentManagers[iter->second].get();
	}

	IComponentManager& EntityRegistry::GetComponentManagerAt(size_t index)
	{
		return *m_ComponentManagers[index];
	}

	const IComponentManager& EntityRegistry::GetComponentManagerAt(size_t index) const
	{
		return *m_ComponentManagers[index];
	}

	bool EntityRegistry::EntityValid(EntityID entity) const
	{
		return entity < m_NextEntityID ? m_EntityAlive.IsSet(entity) : false;
	}

	bool EntityRegistry::EntityActiveHierarchy(EntityID entity) const
	{
		return entity == 0 ? true : (entity < m_NextEntityID ? m_EntityActiveHierarchy.IsSet(entity) : false);
	}

	bool EntityRegistry::EntityActiveSelf(EntityID entity) const
	{
		return entity == 0 ? true : (entity < m_NextEntityID ? m_EntityActiveSelf.IsSet(entity) : false);
	}

	EntityID EntityRegistry::GetParent(EntityID entity) const
	{
		return m_Parents[entity];
	}

	void EntityRegistry::SetParent(EntityID entity, EntityID parent)
	{
		EntityID& oldParent = m_Parents[entity];
		if (oldParent == parent) return;

		/*
		 * Check if the chosen parent is not inside the tree of the entity.
		 * Since we can't just have a parenting loop.
		 */
		EntityID parentParent = parent;
		while (parentParent)
		{
			if (parentParent == entity) return;
			parentParent = m_Parents[parentParent];
		}

		/* Erase from old parent tree */
		auto iter = std::find(m_EntityTrees[oldParent].begin(), m_EntityTrees[oldParent].end(), entity);
		m_EntityTrees[oldParent].erase(iter);

		/* Add to new parent tree */
		oldParent = parent;
		m_EntityTrees[parent].emplace_back(entity);

		/* Component managers for components on this entity will need to be resorted */
		for (size_t i = 0; i < m_ComponentManagers.size(); ++i)
		{
			if (!m_HasComponent[entity].IsSet(i)) continue;
			m_ComponentOrderDirty.Set(i, true);
		}

		/* Update hierarchy active state */
		const bool activeSelf = m_EntityActiveSelf.IsSet(entity);
		const bool activeHierarchy = m_EntityActiveHierarchy.IsSet(entity);
		if (!activeSelf) return;
		if (!parent)
		{
			if (activeHierarchy) return;
			m_EntityActiveHierarchy.Set(entity);
			SetHierarchyActiveStateChildren(entity, true);
			return;
		}

		const bool parentActive = m_EntityActiveHierarchy.IsSet(parent);
		m_EntityActiveHierarchy.Set(entity, parentActive);
		SetHierarchyActiveStateChildren(entity, parentActive);

	}

	void EntityRegistry::Sort()
	{
		if (!m_ComponentOrderDirty.HasAnySet()) return;
		for (size_t i = 0; i < m_ComponentManagers.size(); ++i)
		{
			if (!m_ComponentOrderDirty.IsSet(i)) continue;
			m_ComponentManagers[i]->Sort(m_EntityTrees);
		}
		m_ComponentOrderDirty.Clear();
	}

	void EntityRegistry::SetActive(EntityID entity, bool active, bool withCallbacks)
	{
		const bool wasActive = m_EntityActiveHierarchy.IsSet(entity);
		m_EntityActiveSelf.Set(entity, active);
		m_EntityActiveHierarchy.Set(entity, active);

		if (active)
		{
			EntityID parent = m_Parents[entity];
			const bool hierarchyActive = parent ? m_EntityActiveHierarchy.IsSet(parent) : true;
			m_EntityActiveHierarchy.Set(entity, hierarchyActive);
		}

		const bool isActive = m_EntityActiveHierarchy.IsSet(entity);
		if (wasActive == isActive) return;
		SetHierarchyActiveStateChildren(entity, isActive, withCallbacks);
	}

	void EntityRegistry::DestroyEntity(EntityID entity)
	{
		/* Remove all components */
		Clear(entity);

		/* Update parent */
		EntityID& parent = m_Parents[entity];
		/* Remove from parent children array */
		const auto iter = std::find(m_EntityTrees[parent].begin(), m_EntityTrees[parent].end(), entity);
		m_EntityTrees[parent].erase(iter);

		if (parent)
			SetEntityDirty(parent);

		parent = 0;

		m_EntityActiveSelf.UnSet(entity);
		m_EntityActiveHierarchy.UnSet(entity);
		m_EntityDirty.UnSet(entity);
		m_EntityAlive.UnSet(entity);

		--m_AliveCount;
	}

	void EntityRegistry::Clear(EntityID entity)
	{
		for (size_t i = 0; i < m_ComponentManagers.size(); ++i)
		{
			if (!m_HasComponent[entity].IsSet(i)) continue;
			if (m_ComponentManagers[i]->IsActive(entity) && m_EntityActiveHierarchy.IsSet(entity))
			{
				m_ComponentManagers[i]->CallOnDisableDraw(entity);
				m_ComponentManagers[i]->CallOnDeactivate(entity);
			}
			m_ComponentManagers[i]->CallStop(entity);
			m_ComponentManagers[i]->Remove(entity);
		}
		m_HasComponent[entity].Clear();
		m_EntityComponentOrder[entity].clear();
	}

	bool EntityRegistry::IsEntityDirty(EntityID entity) const
	{
		return m_EntityDirty.IsSet(entity);
	}

	void EntityRegistry::SetEntityDirty(EntityID entity, bool dirty, bool setChildrenDirty, bool withCallbacks)
	{
		const bool wasDirty = m_EntityDirty.IsSet(entity);
		m_EntityDirty.Set(entity, dirty);
		if (wasDirty || !dirty) return;
		for (size_t i = 0; i < m_ComponentManagers.size() && withCallbacks; ++i)
		{
			if (!m_HasComponent[entity].IsSet(i)) continue;
			m_ComponentManagers[i]->CallOnDirty(entity);
		}
		if (!dirty || !setChildrenDirty) return;
		for (size_t i = 0; i < m_EntityTrees[entity].size(); ++i)
		{
			SetEntityDirty(m_EntityTrees[entity][i]);
		}
	}

	size_t EntityRegistry::ChildCount(EntityID entity) const
	{
		return m_EntityTrees[entity].size();
	}

	EntityID EntityRegistry::Child(EntityID entity, size_t index) const
	{
		return m_EntityTrees[entity][index];
	}

	size_t EntityRegistry::SiblingIndex(EntityID entity) const
	{
		const EntityID parent = m_Parents[entity];
		auto iter = std::find(m_EntityTrees[parent].begin(), m_EntityTrees[parent].end(), entity);
		return iter - m_EntityTrees[parent].begin();
	}

	void EntityRegistry::SetSiblingIndex(EntityID entity, size_t index)
	{
		const EntityID parent = m_Parents[entity];
		index = std::min(m_EntityTrees[parent].size() - 1, index);
		auto iter = std::find(m_EntityTrees[parent].begin(), m_EntityTrees[parent].end(), entity);
		size_t currentIndex = iter - m_EntityTrees[parent].begin();
		while (currentIndex != index)
		{
			const size_t nextIndex = currentIndex > index ? currentIndex - 1 : currentIndex + 1;
			std::swap(m_EntityTrees[parent][currentIndex], m_EntityTrees[parent][nextIndex]);
			currentIndex = nextIndex;
		}
		/* If the entity is active, we will need to redorder components for correct execution order later */
		if (!m_EntityActiveHierarchy.IsSet(entity)) return;
		for (size_t i = 0; i < m_ComponentManagers.size(); ++i)
		{
			if (!m_HasComponent[entity].IsSet(i)) continue;
			if (!m_ComponentManagers[i]->IsActive(entity)) continue;
			m_ComponentOrderDirty.Set(i, true);
		}
	}

	size_t EntityRegistry::EntityComponentCount(EntityID entity) const
	{
		return m_EntityComponentOrder[entity].size();
	}

	uint32_t EntityRegistry::EntityComponentType(EntityID entity, size_t index) const
	{
		return m_EntityComponentOrder[entity][index].first;
	}

	UUID EntityRegistry::EntityComponentID(EntityID entity, size_t index) const
	{
		return m_EntityComponentOrder[entity][index].second;
	}

	uint32_t EntityRegistry::EntityComponentIDToHash(EntityID entity, UUID id) const
	{
		auto iter = std::find_if(m_EntityComponentOrder[entity].begin(), m_EntityComponentOrder[entity].end(),
			[this, id](auto& pair) { return pair.second == id; });
		if (iter == m_EntityComponentOrder[entity].end()) return 0;
		return iter->first;
	}

	UUID EntityRegistry::EntityComponentHashToID(EntityID entity, uint32_t typeHash) const
	{
		auto iter = std::find_if(m_EntityComponentOrder[entity].begin(), m_EntityComponentOrder[entity].end(),
			[this, typeHash](auto& pair) { return pair.first == typeHash; });
		if (iter == m_EntityComponentOrder[entity].end()) return 0;
		return iter->second;
	}

	void* EntityRegistry::GetComponentAddress(EntityID entity, uint32_t type)
	{
		IComponentManager* manager = GetComponentManager(type);
		return manager->GetAddress(entity);
	}

	const void* EntityRegistry::GetComponentAddress(EntityID entity, uint32_t type) const
	{
		const IComponentManager* manager = GetComponentManager(type);
		return manager->GetAddress(entity);
	}

	void* EntityRegistry::CopyComponent(EntityID entity, uint32_t type, UUID componentID, const void* data)
	{
		size_t index;
		IComponentManager* manager = GetComponentManager(type, &index);
		void* pAddress = manager->Add(entity, data);
		m_ComponentOrderDirty.Set(index);
		m_HasComponent[entity].Set(index);
		m_EntityComponentOrder[entity].emplace_back(manager->ComponentHash(), componentID);
		manager->CallValidate(entity);
		m_EntityDirty.Set(entity);
		return pAddress;
	}

	EntityID EntityRegistry::CopyEntityToOtherRegistry(EntityID entity, EntityID parent, EntityRegistry* pRegistry) const
	{
		const EntityID newEntity = pRegistry->CreateEntity();

		if (parent) pRegistry->SetParent(newEntity, parent);
		pRegistry->SetActive(newEntity, EntityActiveSelf(entity));
		for (size_t i = 0; i < EntityComponentCount(entity); ++i)
		{
			const uint32_t type = EntityComponentType(entity, i);
			const UUID uuid = EntityComponentID(entity, i);
			const void* data = GetComponentAddress(entity, type);
			pRegistry->CopyComponent(newEntity, type, uuid, data);
		}

		return newEntity;
	}

	void EntityRegistry::SetComponentIndex(EntityID entity, size_t from, size_t to)
	{
		std::vector<std::pair<uint32_t, UUID>>& componentOrder = m_EntityComponentOrder[entity];
		size_t currentIndex = from;
		while (currentIndex != to)
		{
			const size_t nextIndex = currentIndex > to ? currentIndex - 1 : currentIndex + 1;
			std::swap(componentOrder[currentIndex], componentOrder[nextIndex]);
			currentIndex = nextIndex;
		}
	}

	void EntityRegistry::SetUserData(void* data)
	{
		m_pUserData = data;
	}

	void EntityRegistry::EnableCalls()
	{
		m_CallsEnabled = true;
	}

	void EntityRegistry::DisableCalls()
	{
		m_CallsEnabled = false;
	}

	void EntityRegistry::EnableAllIndividualCalls()
	{
		m_EnabledCalls.SetAll();
	}

	size_t EntityRegistry::AliveCount() const
	{
		return m_AliveCount;
	}

	void EntityRegistry::GetReferences(std::vector<UUID>& references) const
	{
		for (auto& manager : m_ComponentManagers)
			manager->GetReferences(references);
	}

	void EntityRegistry::Serialize(BinaryStream& container) const
	{
		for (auto& manager : m_ComponentManagers)
		{
			manager->Serialize(container);
		}
		/* Write a 0 to indicate end of component data */
		container.Write(0u);

		container.Write(m_NextEntityID).Write(m_AliveCount).Write(m_EntityAlive).
			Write(m_EntityActiveSelf).Write(m_EntityActiveHierarchy);

		for (size_t i = 0; i < m_EntityTrees.size(); ++i)
		{
			container.Write(m_EntityTrees[i]);
		}

		container.Write(m_Parents).Write(m_HasComponent);

		for (size_t i = 0; i < m_EntityComponentOrder.size(); ++i)
		{
			container.Write(m_EntityComponentOrder[i]);
		}
	}

	void EntityRegistry::Deserialize(BinaryStream& container)
	{
		uint32_t typeHash;
		size_t blockSize;
		container.Read(typeHash);
		while (typeHash != 0u)
		{
			container.Read(blockSize);

			IComponentManager* pManager = GetComponentManager(typeHash);
			if (!pManager)
			{
				/* Skip block */
				container.Seek(blockSize);

				/* Read next block type hash */
				container.Read(typeHash);
				continue;
			}

			/* Read component manager */
			pManager->Deserialize(container);

			/* Read next block type hash */
			container.Read(typeHash);
		}

		container.Read(m_NextEntityID).Read(m_AliveCount).Read(m_EntityAlive).
			Read(m_EntityActiveSelf).Read(m_EntityActiveHierarchy);

		m_EntityTrees.resize(m_NextEntityID);
		m_Parents.resize(m_NextEntityID);
		m_HasComponent.resize(m_NextEntityID);
		m_EntityComponentOrder.resize(m_NextEntityID);
		for (size_t i = 0; i < m_EntityTrees.size(); ++i)
		{
			container.Read(m_EntityTrees[i]);
		}

		container.Read(m_Parents).Read(m_HasComponent);

		for (size_t i = 0; i < m_EntityComponentOrder.size(); ++i)
		{
			container.Read(m_EntityComponentOrder[i]);
		}

		m_EntityDirty.Reserve(m_NextEntityID);
		m_EntityDirty.SetAll();
	}

	bool EntityRegistry::operator==(const EntityRegistry& other) const
	{
		if (m_NextEntityID != other.m_NextEntityID) return false;
		if (m_AliveCount != other.m_AliveCount) return false;
		if (m_EntityAlive != other.m_EntityAlive) return false;
		if (m_EntityActiveSelf != other.m_EntityActiveSelf) return false;
		if (m_EntityActiveHierarchy != other.m_EntityActiveHierarchy) return false;

		if (m_EntityTrees.size() != other.m_EntityTrees.size() || m_Parents.size() != other.m_Parents.size() ||
			m_HasComponent.size() != other.m_HasComponent.size() ||
			m_EntityComponentOrder.size() != other.m_EntityComponentOrder.size()) return false;

		for (size_t i = 0; i < m_EntityTrees.size(); ++i)
		{
			if (m_EntityTrees[i].size() != other.m_EntityTrees[i].size()) return false;
			if (m_EntityComponentOrder[i].size() != other.m_EntityComponentOrder[i].size()) return false;
			if (m_HasComponent[i] != other.m_HasComponent[i]) return false;
			if (m_Parents[i] != other.m_Parents[i]) return false;

			for (size_t j = 0; j < m_EntityTrees[i].size(); ++j)
				if (m_EntityTrees[i][j] != other.m_EntityTrees[i][j]) return false;

			for (size_t j = 0; j < m_EntityComponentOrder[i].size(); ++j)
				if (m_EntityComponentOrder[i][j] != other.m_EntityComponentOrder[i][j]) return false;
		}

		for (size_t i = 0; i < m_ComponentManagers.size(); ++i)
			if (!m_ComponentManagers[i]->Compare(other.m_ComponentManagers[i].get())) return false;

		return true;
	}

	const std::type_index EntityRegistry::GetComponentType(uint32_t typeHash) const
	{
		const IComponentManager* manager = GetComponentManager(typeHash);
		return manager->ComponentType();
	}

	void EntityRegistry::Reset()
	{
		for (auto& manager : m_ComponentManagers)
			manager->Clear();

		for (size_t i = 0; i < m_EntityTrees.size(); ++i)
		{
			m_EntityTrees[i].clear();
			m_Parents[i] = 0ull;
			m_HasComponent[i].Clear();
			m_EntityComponentOrder[i].clear();
		}

		m_ComponentOrderDirty.Clear();
		m_EntityAlive.Clear();
		m_EntityActiveSelf.Clear();
		m_EntityActiveHierarchy.Clear();
		m_EntityDirty.Clear();

		m_NextEntityID = 1ull;
		m_AliveCount = 0ull;
		m_pUserData = nullptr;
	}

	void EntityRegistry::SetComponentOrderDirty(uint32_t typeHash)
	{
		auto iter = m_HashToComponentManagerIndex.find(typeHash);
		assert(iter != m_HashToComponentManagerIndex.end());
		m_ComponentOrderDirty.Set(iter->second);
	}

	void EntityRegistry::SetComponentOrderDirtyAt(size_t index)
	{
		m_ComponentOrderDirty.Set(index);
	}

	void EntityRegistry::Dirty()
	{
		for (auto& manager : m_ComponentManagers)
			manager->Dirty();
	}

	void EntityRegistry::Validate()
	{
		/* Validate always gets called first before anything else, so perfect time to sort */
		Sort();

		for (auto& manager : m_ComponentManagers)
			manager->Validate();
	}

	void EntityRegistry::Activate()
	{
		for (auto& manager : m_ComponentManagers)
			manager->Activate();
	}

	void EntityRegistry::Deactivate()
	{
		for (auto& manager : m_ComponentManagers)
			manager->Deactivate();
	}

	void EntityRegistry::EnableDraw()
	{
		for (auto& manager : m_ComponentManagers)
			manager->EnableDraw();
	}

	void EntityRegistry::DisableDraw()
	{
		for (auto& manager : m_ComponentManagers)
			manager->DisableDraw();
	}

	void EntityRegistry::Start()
	{
		for (auto& manager : m_ComponentManagers)
			manager->Start();
	}

	void EntityRegistry::Stop()
	{
		for (auto& manager : m_ComponentManagers)
			manager->Stop();
	}

	void EntityRegistry::Update(float dt)
	{
		Sort();

		for (auto& manager : m_ComponentManagers)
			manager->PreUpdate(dt);
		for (auto& manager : m_ComponentManagers)
			manager->Update(dt);
		for (auto& manager : m_ComponentManagers)
			manager->PostUpdate(dt);
	}

	void EntityRegistry::Draw()
	{
		Sort();

		for (auto& manager : m_ComponentManagers)
			manager->PreDraw();
		for (auto& manager : m_ComponentManagers)
			manager->Draw();
		for (auto& manager : m_ComponentManagers)
			manager->PostDraw();
	}

	void EntityRegistry::CallOnValidate(EntityID entity)
	{
		for (size_t i = 0; i < m_ComponentManagers.size(); ++i)
		{
			if (!m_HasComponent[entity].IsSet(i)) continue;
			m_ComponentManagers[i]->CallValidate(entity);
		}
	}

	void EntityRegistry::CallStart(EntityID entity)
	{
		for (size_t i = 0; i < m_ComponentManagers.size(); ++i)
		{
			if (!m_HasComponent[entity].IsSet(i)) continue;
			m_ComponentManagers[i]->CallStart(entity);
		}
	}

	void EntityRegistry::CallOnActivate(EntityID entity)
	{
		for (size_t i = 0; i < m_ComponentManagers.size(); ++i)
		{
			if (!m_HasComponent[entity].IsSet(i)) continue;
			m_ComponentManagers[i]->CallOnActivate(entity);
		}
	}

	void EntityRegistry::CallOnEnableDraw(EntityID entity)
	{
		for (size_t i = 0; i < m_ComponentManagers.size(); ++i)
		{
			if (!m_HasComponent[entity].IsSet(i)) continue;
			m_ComponentManagers[i]->CallOnEnableDraw(entity);
		}
	}

	void EntityRegistry::EnableCall(EntityCallType callType, bool enable)
	{
		m_EnabledCalls.Set(size_t(callType), enable);
	}

	bool EntityRegistry::IsCallEnabled(EntityCallType callType) const
	{
		return m_CallsEnabled && m_EnabledCalls.IsSet(size_t(callType));
	}

	void EntityRegistry::SetHierarchyActiveStateChildren(EntityID entity, bool active, bool withCallbacks)
	{
		/* Component managers for components on this entity will need to be resorted */
		/* We also need to call OnActivate or OnDeactivate on the entity */
		for (size_t i = 0; i < m_ComponentManagers.size() && withCallbacks; ++i)
		{
			if (!m_HasComponent[entity].IsSet(i)) continue;
			/* If the component was not active we don't need to do anything */
			if (!m_ComponentManagers[i]->IsActive(entity)) continue;
			m_ComponentOrderDirty.Set(i, true);
			if (active)
			{
				m_ComponentManagers[i]->CallOnActivate(entity);
				m_ComponentManagers[i]->CallOnEnableDraw(entity);
			}
			else
			{
				m_ComponentManagers[i]->CallOnDeactivate(entity);
				m_ComponentManagers[i]->CallOnDisableDraw(entity);
			}
		}

		/* We should assume that the entity is dirty */
		SetEntityDirty(entity, true, true, withCallbacks);

		for (size_t i = 0; i < m_EntityTrees[entity].size(); ++i)
		{
			EntityID child = m_EntityTrees[entity][i];
			const bool activeSelf = m_EntityActiveSelf.IsSet(child);
			m_EntityActiveHierarchy.Set(child, active && activeSelf);
			SetHierarchyActiveStateChildren(child, active && activeSelf);
		}
	}
}

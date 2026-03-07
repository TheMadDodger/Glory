#include "EntityRegistry.h"

#include <cassert>

namespace Glory::Utils::ECS
{
	EntityRegistry::EntityRegistry(size_t reserveComponentManagers, size_t reserveEntities):
		m_ComponentManagers(), m_ComponentOrderDirty(reserveComponentManagers), m_NextEntityID(1ull)
	{
		m_ComponentManagers.reserve(reserveComponentManagers);
		m_EntityTrees.resize(reserveEntities);
		m_Parents.resize(reserveEntities);
		m_HasComponent.resize(reserveEntities);
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

		EntityID newEntity = m_NextEntityID;
		++m_NextEntityID;

		m_EntityAlive.Set(newEntity, true);
		m_EntityActiveSelf.Set(newEntity, true);
		m_EntityActiveHierarchy.Set(newEntity, true);

		m_EntityTrees.resize(newEntity + 1);
		m_Parents.resize(newEntity + 1);
		m_HasComponent.resize(newEntity + 1);

		m_Parents[newEntity] = 0ull;
		m_EntityTrees[0ull].emplace_back(newEntity);
		m_HasComponent[newEntity].Reserve(m_ComponentManagers.size());
		m_HasComponent[newEntity].Clear();

		return newEntity;
	}

	void EntityRegistry::AddManager(IComponentManager* manager)
	{
		const uint32_t hash = manager->ComponentHash();
		const uint32_t index = uint32_t(m_ComponentManagers.size());
		m_ComponentManagers.emplace_back(manager);
		m_ComponentOrderDirty.Reserve(index + 1ull);
		m_ComponentOrderDirty.Set(index, false);
		m_HashToComponentManagerIndex.emplace(hash, index);
		manager->Initialize();
	}

	IComponentManager* EntityRegistry::GetComponentManager(uint32_t componentHash, size_t* outIndex)
	{
		auto iter = m_HashToComponentManagerIndex.find(componentHash);
		assert(iter != m_HashToComponentManagerIndex.end());
		if (outIndex) *outIndex = iter->second;
		return m_ComponentManagers[iter->second].get();
	}

	bool EntityRegistry::EntityValid(EntityID entity) const
	{
		return entity < m_NextEntityID ? m_EntityAlive.IsSet(entity) : false;
	}

	bool EntityRegistry::EntityActiveHierarchy(EntityID entity) const
	{
		return entity < m_NextEntityID ? m_EntityActiveHierarchy.IsSet(entity) : false;
	}

	bool EntityRegistry::EntityActiveSelf(EntityID entity) const
	{
		return entity < m_NextEntityID ? m_EntityActiveSelf.IsSet(entity) : false;
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
	}

	void EntityRegistry::Sort()
	{
		for (size_t i = 0; i < m_ComponentManagers.size(); ++i)
		{
			if (!m_ComponentOrderDirty.IsSet(i)) continue;
			m_ComponentManagers[i]->Sort(m_EntityTrees);
			m_ComponentOrderDirty.Set(i, false);
		}
	}

	void EntityRegistry::SetActive(EntityID entity, bool active)
	{
		const bool wasActive = m_EntityActiveHierarchy.IsSet(entity);
		m_EntityActiveSelf.Set(entity, active);
		m_EntityActiveHierarchy.Set(entity, active);

		if (active)
		{
			EntityID parent = m_Parents[entity];
			const bool hierarchyActive = parent && !m_EntityActiveHierarchy.IsSet(parent);
			m_EntityActiveHierarchy.Set(entity, hierarchyActive);
		}

		const bool isActive = m_EntityActiveHierarchy.IsSet(entity);
		if (wasActive == isActive) return;
		SetHierarchyActiveStateChildren(entity, isActive);
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
		for (auto& manager : m_ComponentManagers)
			manager->PreDraw();
		for (auto& manager : m_ComponentManagers)
			manager->Draw();
		for (auto& manager : m_ComponentManagers)
			manager->PostDraw();
	}

	void EntityRegistry::SetHierarchyActiveStateChildren(EntityID entity, bool active)
	{
		/* Component managers for components on this entity will need to be resorted */
		for (size_t i = 0; i < m_ComponentManagers.size(); ++i)
		{
			if (!m_HasComponent[entity].IsSet(i)) continue;
			m_ComponentOrderDirty.Set(i, true);
		}

		for (size_t i = 0; i < m_EntityTrees[entity].size(); ++i)
		{
			EntityID child = m_EntityTrees[entity][i];
			m_EntityActiveHierarchy.Set(child, active);
			SetHierarchyActiveStateChildren(child, active);
		}
	}
}

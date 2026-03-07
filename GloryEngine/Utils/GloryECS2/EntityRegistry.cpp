#include "EntityRegistry.h"

#include <cassert>

namespace Glory::Utils::ECS
{
	EntityRegistry::EntityRegistry(size_t reserveComponentManager):
		m_ComponentManagers(), m_NextEntityID(1ull)
	{
		m_ComponentManagers.reserve(reserveComponentManager);
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

		return newEntity;
	}

	IComponentManager* EntityRegistry::GetComponentManager(uint32_t componentHash)
	{
		auto iter = m_HashToComponentManagerIndex.find(componentHash);
		assert(iter != m_HashToComponentManagerIndex.end());
		return m_ComponentManagers[iter->second].get();
	}

	bool EntityRegistry::EntityValid(EntityID entity) const
	{
		return entity < m_NextEntityID ? m_EntityAlive.IsSet(entity) : false;
	}

	bool EntityRegistry::EntityActive(EntityID entity) const
	{
		return entity < m_NextEntityID ? m_EntityActiveSelf.IsSet(entity) &&
			m_EntityActiveHierarchy.IsSet(entity) : false;
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
}

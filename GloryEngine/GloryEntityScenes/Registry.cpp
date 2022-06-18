#include "Entity.h"
#include "Registry.h"
#include <algorithm>
#include "EntityScene.h"

namespace Glory
{
	Registry::Registry(EntityScene* pScene) : m_pScene(pScene)
	{
	}

	Registry::~Registry()
	{
		m_AllEntityIDs.clear();
		m_DeadEntityIndices.clear();
		m_EntityComponents.clear();
		m_UnusedComponentIndices.clear();
		m_ComponentsPerEntity.clear();
		m_ComponentsPerType.clear();
	}

	EntityScene* Registry::GetEntityScene()
	{
		return m_pScene;
	}

	EntityID Registry::CreateEntity()
	{
		if (m_DeadEntityIndices.size() > 0)
		{
			size_t deadEntityIndex = m_DeadEntityIndices[0];
			m_DeadEntityIndices.erase(m_DeadEntityIndices.begin());

			EntityID entity = m_AllEntityIDs[deadEntityIndex];
			return entity;
		}

		size_t index = m_AllEntityIDs.size();
		EntityID newEntity = index + 1;
		m_AllEntityIDs.push_back(newEntity);
		m_ComponentsPerEntity.emplace(newEntity, std::vector<size_t>());
		return newEntity;
	}

	void Registry::DestroyEntity(EntityID entity)
	{
		if (!IsValid(entity)) return;

		Clear(entity);

		size_t index = entity - 1;
		m_DeadEntityIndices.push_back(index);
	}

	void Registry::RemoveComponent(EntityID entity, size_t index)
	{
		std::unique_lock<std::mutex> lock(m_EntityComponentsLock);
		if (!IsValid(entity))
		{
			lock.unlock();
			return;
		}

		if (m_ComponentsPerEntity[entity].size() <= index) return;

		size_t componentIndex = m_ComponentsPerEntity[entity][index];
		EntityComponentData* pComponent = &m_EntityComponents[componentIndex];
		std::type_index type = pComponent->GetType();
		m_Systems.OnComponentRemoved(this, entity, pComponent);
		m_UnusedComponentIndices.push_back(componentIndex);
		m_ComponentsPerEntity[entity].erase(m_ComponentsPerEntity[entity].begin() + index);
		auto it = std::remove(m_ComponentsPerType[type].begin(), m_ComponentsPerType[type].end(), componentIndex);
		m_ComponentsPerType[type].erase(it);
		lock.unlock();
	}

	void Registry::Clear(EntityID entity)
	{
		if (!IsValid(entity)) return;

		for (size_t i = 0; i < m_ComponentsPerEntity[entity].size(); i++)
			m_UnusedComponentIndices.push_back(m_ComponentsPerEntity[entity][i]);

		for (size_t i = 0; i < m_ComponentsPerEntity[entity].size(); i++)
		{
			size_t index = m_ComponentsPerEntity[entity][i];
			std::type_index type = m_EntityComponents[index].GetType();
			m_Systems.OnComponentRemoved(this, entity, &m_EntityComponents[index]);
			std::remove(m_ComponentsPerType[type].begin(), m_ComponentsPerType[type].end(), index);
		}

		m_ComponentsPerEntity[entity].clear();
	}

	size_t Registry::Alive()
	{
		return m_AllEntityIDs.size() - m_DeadEntityIndices.size();
	}

	bool Registry::IsValid(EntityID entity)
	{
		size_t index = entity - 1;
		if (index >= m_AllEntityIDs.size()) return false;

		return entity != 0
			&& std::find(m_DeadEntityIndices.begin(), m_DeadEntityIndices.end(), entity - 1) == m_DeadEntityIndices.end();
	}

	void Registry::ForEach(std::function<void(Registry*, EntityID)> func)
	{
		std::for_each(m_AllEntityIDs.begin(), m_AllEntityIDs.end(), [&](EntityID entity)
		{
			if (!IsValid(entity)) return;
			func(this, entity);
		});
	}

	void Registry::ForEachComponent(EntityID entity, std::function<void(Registry*, EntityID, EntityComponentData*)> func)
	{
		if (!IsValid(entity)) return;
		std::for_each(m_ComponentsPerEntity[entity].begin(), m_ComponentsPerEntity[entity].end(), [&](size_t index)
		{
			func(this, entity, &m_EntityComponents[index]);
		});
	}

	EntityComponentData* Registry::GetEntityComponentDataAt(EntityID entity, size_t index)
	{
		if (m_ComponentsPerEntity.find(entity) == m_ComponentsPerEntity.end()) return nullptr;
		if (m_ComponentsPerEntity[entity].size() <= index) return nullptr;
		size_t componentIndex = m_ComponentsPerEntity[entity][index];
		return &m_EntityComponents[componentIndex];
	}

	void Registry::ForEach(const std::type_index& type, std::function<void(Registry*, EntityID, EntityComponentData*)> func)
	{
		if (m_ComponentsPerType.find(type) == m_ComponentsPerType.end() || m_ComponentsPerType[type].size() <= 0) return;

		std::for_each(m_ComponentsPerType[type].begin(), m_ComponentsPerType[type].end(), [&](size_t index)
		{
			EntityComponentData* pData = &m_EntityComponents[index];
			EntityID entity = pData->m_Entity;
			if (!IsValid(entity)) return;
			func(this, entity, pData);
		});
	}

	void Registry::Update()
	{
		m_Systems.OnUpdate();
	}

	bool Registry::IsUpdating()
	{
		return m_Systems.IsUpdating();
	}

	void Registry::Draw()
	{
		m_Systems.OnDraw();
	}

	EntitySystems* Registry::GetSystems()
	{
		return &m_Systems;
	}
}

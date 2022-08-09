#pragma once
#include <vector>
#include <list>
#include <map>
#include <functional>
#include <Glory.h>
#include "EntityComponentData.h"
#include "EntitySystems.h"

namespace Glory
{
	class Registry
	{
	public:
		GLORY_API Registry();
		GLORY_API virtual ~Registry();

		GLORY_API EntityID CreateEntity();
		GLORY_API void DestroyEntity(EntityID entity);

		template<typename T, typename... Args>
		GLORY_API T& AddComponent(EntityID entity, UUID uuid, Args&&... args)
		{
			if (HasComponent<T>(entity))
				throw new std::runtime_error("Entity already has component!");

			EntityComponentData componentData = EntityComponentData::Construct<T>(entity, uuid, std::forward<Args>(args)...);
			if (m_UnusedComponentIndices.size() > 0)
			{
				// Replace an existing component that is no longer used
				size_t index = m_UnusedComponentIndices[0];
				m_UnusedComponentIndices.erase(m_UnusedComponentIndices.begin());
				m_EntityComponents[index] = componentData;
				m_ComponentsPerEntity[entity].push_back(index);
				m_ComponentsPerType[typeid(T)].push_back(index);
				m_Systems.OnComponentAdded(this, entity, &m_EntityComponents[index]);
				return componentData.GetData<T>();
			}

			// Add as a new component
			size_t index = m_EntityComponents.size();
			m_ComponentsPerEntity[entity].push_back(index);
			m_ComponentsPerType[typeid(T)].push_back(index);
			m_EntityComponents.push_back(componentData);
			m_Systems.OnComponentAdded(this, entity, &m_EntityComponents[index]);
			return componentData.GetData<T>();
		}

		template<typename T>
		GLORY_API bool HasComponent(EntityID entity)
		{
			std::unique_lock<std::mutex> lock(m_EntityComponentsLock);
			return std::find_if(m_ComponentsPerEntity[entity].begin(), m_ComponentsPerEntity[entity].end(),
				[&](size_t index) { return m_EntityComponents[index].GetType() == typeid(T); }) != m_ComponentsPerEntity[entity].end();
		}

		template<typename T>
		GLORY_API T& GetComponent(EntityID entity)
		{
			std::unique_lock<std::mutex> lock(m_EntityComponentsLock);
			auto it = std::find_if(m_ComponentsPerEntity[entity].begin(), m_ComponentsPerEntity[entity].end(),
				[&](size_t index) { return m_EntityComponents[index].GetType() == typeid(T); });

			if (it == m_ComponentsPerEntity[entity].end())
			{
				throw new std::exception("Entity does not have component!");
			}

			size_t index = *it;

			return m_EntityComponents[index].GetData<T>();
		}

		template<typename T>
		GLORY_API void RemoveComponent(EntityID entity)
		{
			std::unique_lock<std::mutex> lock(m_EntityComponentsLock);
			for (size_t i = 0; i < m_ComponentsPerEntity[entity].size(); i++)
			{
				size_t index = m_ComponentsPerEntity[entity][i];
				if (m_EntityComponents[index].GetType() != typeid(T)) continue;
				m_Systems.OnComponentRemoved(this, entity, &m_EntityComponents[index]);
				m_UnusedComponentIndices.push_back(index);
				m_ComponentsPerEntity[entity].erase(m_ComponentsPerEntity[entity].begin() + index);
				auto it = std::remove(m_ComponentsPerType[typeid(T)].begin(), m_ComponentsPerType[typeid(T)].end(), index);
				m_ComponentsPerType[typeid(T)].erase(it);
			}
			lock.unlock();
		}

		GLORY_API void RemoveComponent(EntityID entity, size_t index);

		GLORY_API void Clear(EntityID entity);

		GLORY_API size_t Alive();

		GLORY_API bool IsValid(EntityID entity);

		GLORY_API void ForEach(std::function<void(Registry*, EntityID)> func);
		GLORY_API void ForEachComponent(EntityID entity, std::function<void(Registry*, EntityID, EntityComponentData*)> func);

		GLORY_API EntityComponentData* GetEntityComponentDataAt(EntityID entity, size_t index);

		template<typename C, typename T>
		GLORY_API EntitySystem* RegisterSystem()
		{
			return m_Systems.Register<C, T>(this);
		}

		template<typename T>
		GLORY_API EntitySystem* RegisterSystem()
		{
			return m_Systems.Register<T>(this);
		}

		template<typename T>
		GLORY_API void ForEach(std::function<void(Registry*, EntityID, EntityComponentData*)> func)
		{
			ForEach(typeid(T), func);
		}

		GLORY_API void ForEach(const std::type_index& type, std::function<void(Registry*, EntityID, EntityComponentData*)> func);

		GLORY_API void Update();
		GLORY_API bool IsUpdating();
		GLORY_API void Draw();
		GLORY_API EntitySystems* GetSystems();

	private:
		std::vector<EntityID> m_AllEntityIDs;
		std::vector<size_t> m_DeadEntityIndices;

		std::mutex m_EntityComponentsLock;
		std::vector<EntityComponentData> m_EntityComponents;
		std::vector<size_t> m_UnusedComponentIndices;
		std::map<EntityID, std::vector<size_t>> m_ComponentsPerEntity;
		std::map<std::type_index, std::vector<size_t>> m_ComponentsPerType;

		EntitySystems m_Systems;
	};
}
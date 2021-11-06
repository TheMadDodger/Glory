#pragma once
#include <cstdint>
#include "EntityID.h"
#include <functional>
#include "EntityScene.h"

namespace Glory
{
	class Entity
	{
	public:
		Entity();
		Entity(EntityID entityHandle, EntityScene* pScene);

	public:
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			return m_pEntityScene->m_Registry.AddComponent<T>(m_EntityID, std::forward<Args>(args)...);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_pEntityScene->m_Registry.HasComponent<T>(m_EntityID);
		}

		template<typename T>
		T& GetComponent()
		{
			return m_pEntityScene->m_Registry.GetComponent<T>(m_EntityID);
		}

		template<typename T>
		void RemoveComponent()
		{
			m_pEntityScene->m_Registry.RemoveComponent<T>(m_EntityID);
		}

		void Clear();
		bool IsValid();
		void ForEachComponent(std::function<void(Registry*, EntityID, EntityComponentData*)> func);

		void Destroy();

	private:
		EntityID m_EntityID;
		EntityScene* m_pEntityScene;
	};
}
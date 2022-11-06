#pragma once
#include <cstdint>
#include <functional>
#include <Glory.h>
#include "EntityID.h"
#include "EntityScene.h"

namespace Glory
{
	class Entity
	{
	public:
		GLORY_API Entity();
		GLORY_API Entity(EntityID entityHandle, EntityScene* pScene);

	public:
		template<typename T, typename... Args>
		GLORY_API T& AddComponent(Args&&... args)
		{
			return m_pEntityScene->m_Registry.AddComponent<T>(m_EntityID, std::forward<Args>(args)...);
		}

		template<typename T>
		GLORY_API bool HasComponent()
		{
			return m_pEntityScene->m_Registry.HasComponent<T>(m_EntityID);
		}

		template<typename T>
		GLORY_API T& GetComponent()
		{
			return m_pEntityScene->m_Registry.GetComponent<T>(m_EntityID);
		}

		template<typename T>
		GLORY_API void RemoveComponent()
		{
			m_pEntityScene->m_Registry.RemoveComponent<T>(m_EntityID);
		}

		GLORY_API EntityView* GetEntityView();

		GLORY_API void Clear();
		GLORY_API bool IsValid();
		//GLORY_API void ForEachComponent(std::function<void(EntityRegistry*, EntityID, EntityComponentData*)> func);

		GLORY_API void Destroy();

		GLORY_API EntityID GetEntityID();

		GLORY_API EntityScene* GetScene();

	private:
		EntityID m_EntityID;
		EntityScene* m_pEntityScene;
	};
}
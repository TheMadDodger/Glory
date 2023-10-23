#pragma once
#include <cstdint>
#include <functional>
#include <GloryECS/EntityID.h>
#include <GloryECS/EntityRegistry.h>

namespace Glory
{
	class GScene;

	class Entity
	{
	public:
		Entity();
		Entity(Utils::ECS::EntityID entityHandle, GScene* pScene);

	public:
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			return m_pRegistry->AddComponent<T>(m_EntityID, std::forward<Args>(args)...);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_pRegistry->HasComponent<T>(m_EntityID);
		}

		template<typename T>
		T& GetComponent()
		{
			return m_pRegistry->GetComponent<T>(m_EntityID);
		}

		template<typename T>
		void RemoveComponent()
		{
			m_pRegistry->RemoveComponent<T>(m_EntityID);
		}

		Utils::ECS::EntityView* GetEntityView();

		void Clear();
		bool IsValid();
		//GLORY_API void ForEachComponent(std::function<void(EntityRegistry*, EntityID, EntityComponentData*)> func);

		void Destroy();

		Utils::ECS::EntityID GetEntityID();

		Utils::ECS::EntityRegistry* GetRegistry();
		GScene* GetScene();

		bool IsActiveSelf() const;
		bool IsActive() const;
		void SetActive(bool active);
		void SetActiveHierarchy(bool active);

	private:
		Utils::ECS::EntityID m_EntityID;
		Utils::ECS::EntityRegistry* m_pRegistry;
		GScene* m_pGScene;
	};
}
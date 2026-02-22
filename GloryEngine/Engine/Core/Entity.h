#pragma once
#include <cstdint>
#include <functional>
#include <EntityID.h>
#include <EntityRegistry.h>

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

		Entity ParentEntity() const;
		Utils::ECS::EntityID Parent() const;
		void SetParent(Utils::ECS::EntityID parent);

		Utils::ECS::EntityView* GetEntityView() const;

		void Clear();
		bool IsValid() const;
		bool IsDirty() const;
		void SetDirty(bool dirty=true);
		void Destroy();

		Utils::ECS::EntityID GetEntityID() const;
		UUID EntityUUID() const;

		size_t ChildCount() const;
		Entity ChildEntity(size_t index) const;
		Utils::ECS::EntityID Child(size_t index) const;
		size_t SiblingIndex() const;
		void SetSiblingIndex(size_t index);

		Utils::ECS::EntityRegistry* GetRegistry() const;
		GScene* GetScene() const;

		bool IsActiveSelf() const;
		bool IsActive() const;
		bool IsHierarchyActive() const;
		void SetActive(bool active);
		void SetActiveSelf(bool active);
		void SetActiveHierarchy(bool active);

		std::string_view Name() const;
		void UpdateHierarchyActive();

	private:
		Utils::ECS::EntityID m_EntityID;
		Utils::ECS::EntityRegistry* m_pRegistry;
		GScene* m_pGScene;
	};
}
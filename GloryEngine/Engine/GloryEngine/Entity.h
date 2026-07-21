#pragma once
#include <UUID.h>
#include <EntityRegistry.h>

#include <engine_visibility.h>

#include <cstdint>
#include <functional>
#include <string_view>

namespace Glory
{
	class GScene;

	class Entity
	{
	public:
		GLORY_ENGINE_API Entity();
		GLORY_ENGINE_API Entity(Utils::ECS::EntityID entityHandle, GScene* pScene);

	public:
		template<typename T, typename... Args>
		inline T& AddComponent(Args&&... args)
		{
			return m_pRegistry->AddComponent<T>(m_EntityID, UUID(), std::forward<Args>(args)...);
		}

		template<typename T>
		inline bool HasComponent() const
		{
			return m_pRegistry->HasComponent<T>(m_EntityID);
		}

		template<typename T>
		inline T& GetComponent()
		{
			return m_pRegistry->GetComponent<T>(m_EntityID);
		}

		template<typename T>
		inline const T& GetComponent() const
		{
			return m_pRegistry->GetComponent<T>(m_EntityID);
		}

		template<typename T>
		inline void RemoveComponent()
		{
			m_pRegistry->RemoveComponent<T>(m_EntityID);
		}

		GLORY_ENGINE_API Entity ParentEntity() const;
		GLORY_ENGINE_API Utils::ECS::EntityID Parent() const;
		GLORY_ENGINE_API void SetParent(Utils::ECS::EntityID parent);

		GLORY_ENGINE_API void Clear();
		GLORY_ENGINE_API bool IsValid() const;
		GLORY_ENGINE_API bool IsDirty() const;
		GLORY_ENGINE_API void SetDirty(bool dirty=true);
		GLORY_ENGINE_API void Destroy();

		GLORY_ENGINE_API Utils::ECS::EntityID GetEntityID() const;
		GLORY_ENGINE_API UUID EntityUUID() const;

		GLORY_ENGINE_API size_t ChildCount() const;
		GLORY_ENGINE_API Entity ChildEntity(size_t index) const;
		GLORY_ENGINE_API Utils::ECS::EntityID Child(size_t index) const;
		GLORY_ENGINE_API size_t SiblingIndex() const;
		GLORY_ENGINE_API void SetSiblingIndex(size_t index);

		GLORY_ENGINE_API Utils::ECS::EntityRegistry* GetRegistry() const;
		GLORY_ENGINE_API GScene* GetScene() const;

		GLORY_ENGINE_API bool IsActiveSelf() const;
		GLORY_ENGINE_API bool IsActive() const;
		GLORY_ENGINE_API bool IsHierarchyActive() const;
		GLORY_ENGINE_API void SetActive(bool active, bool withCallbacks=true);

		GLORY_ENGINE_API std::string_view Name() const;

		GLORY_ENGINE_API size_t ComponentCount() const;
		GLORY_ENGINE_API uint32_t ComponentType(size_t index) const;
		GLORY_ENGINE_API size_t ComponentID(size_t index) const;

	private:
		Utils::ECS::EntityID m_EntityID;
		Utils::ECS::EntityRegistry* m_pRegistry;
		GScene* m_pGScene;
	};
}
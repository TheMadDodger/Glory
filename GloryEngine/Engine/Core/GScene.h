#pragma once
#include "Resource.h"
#include "Entity.h"

#include <EntityRegistry.h>

namespace Glory
{
	namespace Utils::ECS
	{
		class EntityRegistry;
	}

	class GScene : public Resource
	{
	public:
		/** @brief Constructor */
		GScene(const std::string& sceneName = "New Scene");
		/** @overload */
		GScene(const std::string& sceneName, UUID uuid);
		/** @brief Destructor */
		virtual ~GScene();

	public:
		/** @brief Create a new entity with an ID and transform with ID
		 * @param uuid ID of the entity, not the same as EntityID
		 * @param transUuid ID of the entities transform component
		 */
		Entity CreateEmptyObject(UUID uuid = UUID(), UUID transUuid = UUID());
		/** @overload
		 * @param name Name of the entity
		 * @param uuid ID of the entity, not the same as EntityID
		 * @param transUuid ID of the entities transform component
		 */
		Entity CreateEmptyObject(const std::string& name, UUID uuid = UUID(), UUID transUuid = UUID());

		/** @brief Get number of active entities in this scene */
		size_t SceneObjectsCount() const;

		/** @brief Get an entity by UUID */
		Entity GetEntity(UUID uuid);
		/** @brief Get an entity by entity ID */
		Entity GetEntityByEntityID(Utils::ECS::EntityID entityId);

		/** @brief Invoke a start on all entities and components */
		void Start();
		/** @brief Invoke a stop on all entities and components */
		void Stop();

		/** @brief Get prefab ID for an entities UUID */
		const UUID Prefab(UUID objectID) const;
		/** @brief Get prefab ID for a child of an entity by UUID */
		const UUID PrefabChild(UUID objectID) const;

		/** @brief Get entity registry for this scene */
		Utils::ECS::EntityRegistry& GetRegistry();

		/** @brief Get the UUID of an entity */
		UUID GetEntityUUID(Utils::ECS::EntityID entity) const;

	protected:
		/** @brief Invoke an update on all active entities and components */
		void OnTick();
		/** @brief Invoke a draw on all active entities and components */
		void OnPaint();

	private:
		/** @brief Set the id of this scene */
		void SetUUID(UUID uuid);
		/** @brief Create an entity with transform component in the registry */
		Entity CreateEntity(UUID transUUID);

	protected:
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneManager;
		std::map<UUID, Utils::ECS::EntityID> m_Ids;
		std::map<Utils::ECS::EntityID, UUID> m_UUIds;
		std::map<Utils::ECS::EntityID, std::string> m_Names;
		std::map<UUID, UUID> m_ActivePrefabs;
		std::map<UUID, UUID> m_ActivePrefabChildren;

		Utils::ECS::EntityRegistry m_Registry;
	};
}

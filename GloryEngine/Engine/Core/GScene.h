#pragma once
#include "Resource.h"
#include "Entity.h"
#include "GraphicsEnums.h"

#include <glm/fwd.hpp>
#include <EntityRegistry.h>

namespace Glory
{
	class PrefabData;
	class SceneManager;
	struct UUIDRemapper;

	namespace Utils::ECS
	{
		class EntityRegistry;
	}

	struct DelayedParentData
	{
		DelayedParentData(Entity pObjectToParent, UUID parentID) : ObjectToParent(pObjectToParent), ParentID(parentID) {}
		
		Entity ObjectToParent;
		UUID ParentID;
	};

	struct SceneSettings
	{
		LightingSettings m_LightingSettings;
		SSAOSettings m_SSAOSettings;
		bool m_DepthWrite{ true };
		bool m_RenderLate{ false };
	};

	/** @brief Glory Scene */
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
		Entity GetEntityByUUID(UUID uuid);
		/** @brief Create an entity handle for an entity in this scene */
		Entity GetEntityByEntityID(Utils::ECS::EntityID entityId);

		/** @brief Destroy an entity
		 * @param entity Entity ID
		 * 
		 * This removes it from the registry and internal ID caches
		 * Also deletes the entire child hierarchy under this entity
		 */
		void DestroyEntity(Utils::ECS::EntityID entity);

		/** @brief Invoke a start on all entities and components */
		void Start();
		/** @brief Invoke a stop on all entities and components */
		void Stop();

		/** @brief Mark an entity in the scene as prefab
		 * and its child hierarchy as prefab children.
		 * @param entity Entity ID
		 * @param prefabID ID of the prefab
		 */
		void SetPrefab(Utils::ECS::EntityID entity, UUID prefabID);
		/** @brief Unmark an entity and in the scene as prefab
		 * and its child hierarchy as prefab children.
		 * @param entity Entity ID
		 */
		void UnsetPrefab(Utils::ECS::EntityID entity);

		/** @brief Get prefab ID for an entities UUID */
		const UUID Prefab(UUID objectID) const;
		/** @brief Get prefab ID for a child of an entity by UUID */
		const UUID PrefabChild(UUID objectID) const;

		/** @brief Get entity registry for this scene */
		Utils::ECS::EntityRegistry& GetRegistry();

		/** @brief Get the UUID of an entity */
		UUID GetEntityUUID(Utils::ECS::EntityID entity) const;

		/** @brief Get the parent entity of an entity */
		Utils::ECS::EntityID Parent(Utils::ECS::EntityID entity) const;

		/** @overload */
		Utils::ECS::EntityID Parent(UUID uuid) const;

		/** @brief Re-parent an entity
		 * @param entity The entity to re-parent
		 * @param parent The entity to parent to, use 0 to unparrent
		 */
		void SetParent(Utils::ECS::EntityID entity, Utils::ECS::EntityID parent);

		/** @brief Get number of children on an entity
		 * @param entity ID of the entity, set to 0 to get the child count on the root
		 */
		size_t ChildCount(Utils::ECS::EntityID entity) const;

		/** @brief Get a child entity by index on an entity
		 * @param entity ID of the entity, set to 0 to get the child on the root
		 * @param index Index of the child
		 * @returns @ref Utils::ECS::EntityID of the child entity,
		 * you will need to check if the entity is valid
		 */
		Utils::ECS::EntityID Child(Utils::ECS::EntityID entity, size_t index) const;

		/** @brief Get entity handle of a child entity
		 *
		 * See @ref GScene::Child() for more info.
		 */
		Entity ChildEntity(Utils::ECS::EntityID entity, size_t index);

		/** @brief Get the sibling index of an entity */
		size_t SiblingIndex(Utils::ECS::EntityID entity) const;

		/** @brief Set the sibling index of an entity */
		void SetSiblingIndex(Utils::ECS::EntityID entity, size_t index);

		/** @brief Get the name of an entity */
		std::string_view EntityName(Utils::ECS::EntityID entity) const;

		/** @brief Set the name of an entity */
		void SetEntityName(Utils::ECS::EntityID entity, const std::string_view name);

		/** @brief Create a new Entity in the scene from a prefab
		 * @param parent The entity to parent the created object to
		 * @param pPrefab The prefab to instantiate
		 * @parem pos Position of the created entity
		 * @parem rot Rotation of the created entity
		 * @parem scale Scale of the created entity
		 */
		Entity InstantiatePrefab(UUID parent, PrefabData* pPrefab,
		    const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale);
		/** @overload
		 * @param parent The entity to parent the created object to
		 * @param pPrefab The prefab to instantiate
		 * @param remapSeed The seed to use to regenerate ids
		 * @parem pos Position of the created entity
		 * @parem rot Rotation of the created entity
		 * @parem scale Scale of the created entity
		 */
		Entity InstantiatePrefab(UUID parent, PrefabData* pPrefab, uint32_t remapSeed,
		    const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale);
		/** @overload
		 * @param parent The entity to parent the created object to
		 * @param pPrefab The prefab to instantiate
		 * @param remapper The ID remapper to generate new ids with
		 * @parem pos Position of the created entity
		 * @parem rot Rotation of the created entity
		 * @parem scale Scale of the created entity
		 */
		Entity InstantiatePrefab(UUID parent, PrefabData* pPrefab, UUIDRemapper& remapper,
		    const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale);

		/** @brief Set the parent of an entity at the next @ref HandleDelayedParents() call
		 *
		 * Used when an entity doesn't exist yet but will exist in the future
		 */
		void DelayedSetParent(Entity entity, UUID parentID);

		/** @brief Handle all delayed parents requested by @ref DelayedSetParent() calls. */
		void HandleDelayedParents();

		/** @brief Get the @ref SceneManager that owns this scene */
		SceneManager* Manager();

		/** @brief Set the @ref SceneManager for this scene
		 * this does not add the scene to the manager.
		 * @param pManager The manager
		 */
		void SetManager(SceneManager* pManager);

		virtual void Serialize(BinaryStream& container) const;
		virtual void Deserialize(BinaryStream& container);

		/** @brief Mark this scene for destruction to prevent further update() and draw() calls */
		void MarkForDestruction();

		Entity Instantiate(GScene* pOther, UUIDRemapper& IDRemapper, Utils::ECS::EntityID parent = 0);

		/** @brief Get this scenes settings */
		SceneSettings& Settings() { return m_Settings; }

		/** @overload */
		const SceneSettings& Settings() const { return m_Settings; }

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

		/** @brief Mark an entity and its entire child hierarchy
		 * in the scene as prefab children
		 * @param entity Entity ID
		 * @param prefabID ID of the prefab
		 */
		void SetChildrenPrefab(Utils::ECS::EntityID entity, UUID prefabID);
		/** @brief Unmark an entity and its entire child hierarchy
		 * in the scene as prefab children
		 * @param entity Entity ID
		 * @param prefabID ID of the prefab
		 */
		void UnsetChildrenPrefab(Utils::ECS::EntityID entity);

		Entity InstantiateEntity(GScene* pOther, UUIDRemapper& IDRemapper, Utils::ECS::EntityID entity,
			std::vector<Utils::ECS::EntityID>& newEntities, Utils::ECS::EntityID parent = 0);

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

		std::vector<DelayedParentData> m_DelayedParents;

		SceneManager* m_pManager = nullptr;

		bool m_MarkedForDestruct = false;

		SceneSettings m_Settings;
	};
}

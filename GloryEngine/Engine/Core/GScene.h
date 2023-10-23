#pragma once
#include "Resource.h"
#include "SceneObject.h"
#include "UUIDRemapper.h"
#include "Entity.h"

#include <mutex>
#include <glm/fwd.hpp>
#include <GloryECS/EntityRegistry.h>

namespace Glory
{
    struct PrefabNode;
    class PrefabData;
    class SceneObject;

    struct DelayedParentData
    {
        DelayedParentData(SceneObject* pObjectToParent, UUID parentID) : ObjectToParent(pObjectToParent), ParentID(parentID) {}

        SceneObject* ObjectToParent;
        UUID ParentID;
    };

    class GScene : public Resource
    {
    public:
        GScene(const std::string& sceneName = "New Scene");
        GScene(const std::string& sceneName, UUID uuid);
        virtual ~GScene();

    public:
        SceneObject* CreateEmptyObject();
        SceneObject* CreateEmptyObject(const std::string& name, UUID uuid, UUID uuid2 = UUID());
        size_t SceneObjectsCount();
        SceneObject* GetSceneObject(size_t index);
        void DeleteObject(SceneObject* pObject);

        SceneObject* FindSceneObject(UUID uuid) const;
        void DelayedSetParent(SceneObject* pObjectToParent, UUID parentID);

        void HandleDelayedParents();

        void Start();
        void Stop();

        void SetPrefab(SceneObject* pObject, UUID prefabID);
        void UnsetPrefab(SceneObject* pObject);
        const UUID Prefab(UUID objectID) const;
        const UUID PrefabChild(UUID objectID) const;

        SceneObject* InstantiatePrefab(SceneObject* pParent, PrefabData* pPrefab,
            const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale);
        SceneObject* InstantiatePrefab(SceneObject* pParent, PrefabData* pPrefab, uint32_t remapSeed,
            const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale);
        SceneObject* InstantiatePrefab(SceneObject* pParent, PrefabData* pPrefab, UUIDRemapper& remapper,
            const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale);

        SceneObject* GetSceneObjectFromEntityID(Utils::ECS::EntityID entity);

        Utils::ECS::EntityRegistry* GetRegistry();

        bool IsValid() const;

    protected:
        void Initialize();
        void OnTick();
        void OnPaint();

        SceneObject* CreateObject(const std::string& name);
        SceneObject* CreateObject(const std::string& name, UUID uuid, UUID uuid2 = 0);
        void OnDeleteObject(SceneObject* pObject);
        void OnObjectAdded(SceneObject* pObject);
        void OnDelayedSetParent(const DelayedParentData& data);

    private:
        void SetUUID(UUID uuid);
        void SetChildrenPrefab(SceneObject* pObject, UUID prefabID);
        void UnsetChildrenPrefab(SceneObject* pObject);

        Entity CreateEntity(UUID uuid, UUID transUUID);

        SceneObject* InstantiatePrefabNode(SceneObject* pParent, const PrefabNode& node, UUIDRemapper& remapper);

    protected:
        friend class Entity;
        friend class SceneSerializer;
        friend class SceneManager;
        friend class SceneObject;
        std::vector<SceneObject*> m_pSceneObjects;
        std::vector<DelayedParentData> m_DelayedParents;
        std::map<UUID, UUID> m_ActivePrefabs;
        std::map<UUID, UUID> m_ActivePrefabChildren;

        Utils::ECS::EntityRegistry m_Registry;
        bool m_Valid;
        std::unordered_map<Utils::ECS::EntityID, SceneObject*> m_EntityIDToObject;
    };
}

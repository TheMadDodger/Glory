#pragma once
#include "Resource.h"
#include "SceneObject.h"
#include <mutex>

namespace Glory
{
    class PrefabData;

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

        virtual void Start() {}
        virtual void Stop() {}

        void SetPrefab(SceneObject* pObject, UUID prefabID);
        const UUID Prefab(UUID objectID) const;
        const UUID PrefabChild(UUID objectID) const;

        virtual SceneObject* InstantiatePrefab(SceneObject* pParent, PrefabData* pPrefab) { return nullptr; }

    protected:
        virtual void Initialize() {}
        virtual void OnTick() {}
        virtual void OnPaint() {}

        virtual SceneObject* CreateObject(const std::string& name) { return nullptr; }
        virtual SceneObject* CreateObject(const std::string& name, UUID uuid, UUID uuid2 = 0) { return nullptr; }
        virtual void OnDeleteObject(SceneObject* pObject) {}
        virtual void OnObjectAdded(SceneObject* pObject) {}
        virtual void OnDelayedSetParent(const DelayedParentData& data);

    private:
        void SetUUID(UUID uuid);
        void SetChildrenPrefab(SceneObject* pObject, UUID prefabID);

    protected:
        friend class ScenesModule;
        friend class SceneObject;
        std::vector<SceneObject*> m_pSceneObjects;
        std::vector<DelayedParentData> m_DelayedParents;
        std::map<UUID, UUID> m_ActivePrefabs;
        std::map<UUID, UUID> m_ActivePrefabChildren;
    };
}

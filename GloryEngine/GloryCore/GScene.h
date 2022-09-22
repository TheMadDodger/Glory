#pragma once
#include "Resource.h"
#include "SceneObject.h"
#include <mutex>

namespace Glory
{
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
        SceneObject* CreateEmptyObject(const std::string& name, UUID uuid);
        size_t SceneObjectsCount();
        SceneObject* GetSceneObject(size_t index);
        void DeleteObject(SceneObject* pObject);

        SceneObject* FindSceneObject(UUID uuid) const;
        void DelayedSetParent(SceneObject* pObjectToParent, UUID parentID);

        void HandleDelayedParents();

    protected:
        virtual void Initialize() {};
        virtual void OnTick() {};
        virtual void OnPaint() {};

        virtual SceneObject* CreateObject(const std::string& name) { return nullptr; };
        virtual SceneObject* CreateObject(const std::string& name, UUID uuid) { return nullptr; };
        virtual void OnDeleteObject(SceneObject* pObject) {};

        virtual void OnObjectAdded(SceneObject* pObject) {};

        virtual void OnDelayedSetParent(const DelayedParentData& data);

    private:
        void SetUUID(UUID uuid);

    protected:
        friend class ScenesModule;
        friend class SceneObject;
        std::vector<SceneObject*> m_pSceneObjects;
        std::vector<DelayedParentData> m_DelayedParents;
    };
}

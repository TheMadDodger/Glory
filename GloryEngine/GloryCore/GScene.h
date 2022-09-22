#pragma once
#include "Resource.h"
#include "SceneObject.h"
#include <mutex>

namespace Glory
{
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

    protected:
        virtual void Initialize() {};
        virtual void OnTick() {};
        virtual void OnPaint() {};

        virtual SceneObject* CreateObject(const std::string& name) { return nullptr; };
        virtual SceneObject* CreateObject(const std::string& name, UUID uuid) { return nullptr; };
        virtual void OnDeleteObject(SceneObject* pObject) {};

        virtual void OnObjectAdded(SceneObject* pObject) {};

    private:
        void SetUUID(UUID uuid);

    protected:
        friend class ScenesModule;
        friend class SceneObject;
        std::vector<SceneObject*> m_pSceneObjects;
    };
}

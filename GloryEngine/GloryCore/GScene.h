#pragma once
#include "Resource.h"
#include "SceneObject.h"
#include <mutex>

namespace Glory
{
    class GScene : public Resource
    {
    public:
        GScene();
        GScene(const std::string& sceneName = "New Scene");
        GScene(const std::string& sceneName, UUID uuid);
        virtual ~GScene();

    public:
        SceneObject* CreateEmptyObject();
        SceneObject* CreateEmptyObject(const std::string& name, UUID uuid);
        size_t SceneObjectsCount();
        SceneObject* GetSceneObject(size_t index);
        const std::string& Name();

    protected:
        virtual void Initialize() = 0;
        virtual void OnTick() = 0;
        virtual void OnPaint() = 0;

        virtual SceneObject* CreateObject(const std::string& name) = 0;
        virtual SceneObject* CreateObject(const std::string& name, UUID uuid) = 0;

        virtual void OnObjectAdded(SceneObject* pObject) {};

    private:
        void SetUUID(UUID uuid);

    protected:
        friend class ScenesModule;
        friend class SceneObject;
        std::string m_SceneName;
        std::vector<SceneObject*> m_pSceneObjects;
    };
}

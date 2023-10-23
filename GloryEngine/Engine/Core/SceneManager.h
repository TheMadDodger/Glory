#pragma once
#include "CameraRef.h"

#include <mutex>

namespace Glory
{
    class Engine;
    class GScene;
    class SceneObject;
    class PrefabDataLoader;
    class SceneObject;

    namespace Utils::ECS
    {
        class ComponentTypes;
    }

    class SceneManager
    {
    public:
        SceneManager(Engine* pEngine);
        virtual ~SceneManager();

    public:
        GScene* CreateEmptyScene(const std::string& name = "Empty Scene");

        size_t OpenScenesCount();
        GScene* GetOpenScene(size_t index);
        GScene* GetOpenScene(UUID uuid);
        GScene* GetActiveScene();
        void SetActiveScene(GScene* pScene);
        void CloseAllScenes();
        void OpenScene(const std::string& path, UUID uuid = UUID());
        void AddOpenScene(GScene* pScene, UUID uuid = 0);
        void CloseScene(UUID uuid);

        void SetHoveringObject(uint64_t objectID);
        SceneObject* GetHoveringObject();

        virtual SceneObject* GetSceneObjectFromObjectID(uint64_t objectID);

        Glory::Utils::ECS::ComponentTypes* ComponentTypesInstance() const;

        template<typename T>
        void RegisterComponent(bool allowMultiple = false, const uint64_t customFlags = 0)
        {
            Glory::Utils::ECS::ComponentTypes::RegisterComponent<T>(allowMultiple, customFlags);
            Reflect::RegisterType<T>();
        }

    protected:
        virtual void Initialize();
        virtual void Cleanup();

        static SceneObject* CreateObject(GScene* pScene, const std::string& name, UUID uuid);

        virtual void OnSceneOpen(UUID uuid) {}
        virtual void OnSceneClose(UUID uuid) {}

    private:
        virtual void Update();
        virtual void Draw();

        static SceneObject* CreateDeserializedObject(GScene* pScene, const std::string& name, UUID uuid);

    private:
        friend class GameThread;
        friend class SceneObjectSerializer;
        friend class SceneSerializer;

    protected:
        Engine* m_pEngine;
        std::vector<GScene*> m_pOpenScenes;
        size_t m_ActiveSceneIndex;
        std::mutex m_HoveringLock;
        SceneObject* m_pHoveringObject;

        Glory::Utils::ECS::ComponentTypes* m_pComponentTypesInstance;

        PrefabDataLoader* m_pPrefabLoader;
    };
}

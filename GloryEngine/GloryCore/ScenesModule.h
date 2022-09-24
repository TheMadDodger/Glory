#pragma once
#include "Module.h"
#include "GScene.h"
#include "CameraRef.h"
#include <mutex>
#include <glm/glm.hpp>

namespace Glory
{
    class ScenesModule : public Module
    {
    public:
        ScenesModule();
        virtual ~ScenesModule();

    public:
        virtual const std::type_info& GetModuleType() override;

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

        void SetHoveringObject(uint32_t objectID);
        SceneObject* GetHoveringObject();

    protected:
        virtual void Initialize() = 0;
        virtual void Cleanup() override;
        virtual void OnCleanup() = 0;

        virtual GScene* CreateScene(const std::string& sceneName) = 0;
        virtual GScene* CreateScene(const std::string& sceneName, UUID uuid) = 0;

        static SceneObject* CreateObject(GScene* pScene, const std::string& name, UUID uuid);

        virtual SceneObject* GetSceneObjectFromObjectID(uint32_t objectID) = 0;

    private:
        virtual void Update() override;
        virtual void Draw() override;

    private:
        friend class GameThread;

    protected:
        std::vector<GScene*> m_pOpenScenes;
        size_t m_ActiveSceneIndex;
        std::mutex m_HoveringLock;
        SceneObject* m_pHoveringObject;
    };
}

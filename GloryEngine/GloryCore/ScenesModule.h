#pragma once
#include "Module.h"
#include "GScene.h"
#include <mutex>

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
        void CloseAllScenes();
        void OpenScene(const std::string& path, UUID uuid = UUID());
        void CloseScene(UUID uuid);

    protected:
        virtual void Initialize() = 0;
        virtual void Cleanup() override;
        virtual void OnCleanup() = 0;

        virtual GScene* CreateScene(const std::string& sceneName) = 0;
        virtual GScene* CreateScene(const std::string& sceneName, UUID uuid) = 0;

        static SceneObject* CreateObject(GScene* pScene, const std::string& name, UUID uuid);

    private:
        virtual void Update() override;
        virtual void Draw() override;

    private:
        friend class GameThread;

    protected:
        std::vector<GScene*> m_pOpenScenes;
        size_t m_ActiveSceneIndex;
    };
}

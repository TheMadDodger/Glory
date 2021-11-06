#pragma once
#include "Module.h"
#include "GScene.h"

namespace Glory
{
    class ScenesModule : public Module
    {
    public:
        virtual const std::type_info& GetModuleType() override;

        GScene* CreateEmptyScene();

        size_t OpenScenesCount();
        GScene* GetOpenScene(size_t index);
        GScene* GetActiveScene();

    protected:
        virtual void Initialize() = 0;
        virtual void Cleanup() override;
        virtual void OnCleanup() = 0;

        virtual GScene* CreateScene(const std::string& sceneName) = 0;
        virtual GScene* CreateScene(const std::string& sceneName, UUID uuid) = 0;

    private:
        void Tick();
        void Paint();

    private:
        friend class GameThread;

    protected:
        std::vector<GScene*> m_pOpenScenes;
        size_t m_ActiveSceneIndex;
    };
}

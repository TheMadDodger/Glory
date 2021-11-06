#pragma once
#include "Resource.h"

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
        const std::string& Name();

    protected:
        virtual void Initialize() = 0;
        virtual void OnTick() = 0;
        virtual void OnPaint() = 0;

    private:
        friend class ScenesModule;
        std::string m_SceneName;
    };
}

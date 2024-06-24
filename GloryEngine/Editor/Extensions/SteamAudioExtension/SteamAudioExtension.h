#pragma once
#include <BaseEditorExtension.h>
#include <IPlayModeHandler.h>

EXTENSION_H

namespace Glory
{
    class Module;
}

namespace Glory::Editor
{
    class SteamAudioExtension : public BaseEditorExtension, public IPlayModeHandler
    {
    public:
        virtual const char* ModuleName() override { return "SDL Audio"; };
        virtual void HandleBeforeStart(Module* pModule) override {};
        virtual void HandleStart(Module* pModule) override {};
        virtual void HandleStop(Module* pModule) override;
        virtual void HandleUpdate(Module* pModule) override {};

        SteamAudioExtension();
        virtual ~SteamAudioExtension();

    private:
        virtual void Initialize() override;
    };
}

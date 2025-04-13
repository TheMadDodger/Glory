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
    class LocalizeEditorExtension : public BaseEditorExtension, IPlayModeHandler
    {
    public:
        LocalizeEditorExtension();
        virtual ~LocalizeEditorExtension();

    private:
        virtual void Initialize() override;

        virtual const char* ModuleName() override;
        virtual void HandleBeforeStart(Module* pModule) override;
        virtual void HandleStart(Module* pModule) override;
        virtual void HandleStop(Module* pModule) override;
        virtual void HandleUpdate(Module* pModule) override;
    };
}

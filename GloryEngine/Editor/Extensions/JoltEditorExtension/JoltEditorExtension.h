#pragma once
#include <BaseEditorExtension.h>
#include <IPlayModeHandler.h>

EXTENSION_H

namespace Glory::Editor
{
    class JoltEditorExtension : public BaseEditorExtension, IPlayModeHandler
    {
    public:
        JoltEditorExtension();
        virtual ~JoltEditorExtension();

    private:
        virtual void Initialize() override;

        virtual const char* ModuleName() override;
        virtual void HandleBeforeStart(Module* pModule) override;
        virtual void HandleStart(Module* pModule) override;
        virtual void HandleStop(Module* pModule) override;
        virtual void HandleUpdate(Module* pModule) override;
    };
}

#pragma once
#include <BaseEditorExtension.h>
#include <IPlayModeHandler.h>

extern "C" GLORY_EDITOR_EXTENSION_API Glory::Editor::BaseEditorExtension* LoadExtension();
extern "C" GLORY_EDITOR_EXTENSION_API void SetContext(Glory::GloryContext * pContext, ImGuiContext * pImGUIContext);

namespace Glory::Editor
{
    class JoltEditorExtension : public BaseEditorExtension, IPlayModeHandler
    {
    public:
        JoltEditorExtension();
        virtual ~JoltEditorExtension();

    private:
        virtual void RegisterEditors() override;

        virtual const char* ModuleName() override;
        virtual void HandleBeforeStart(Module* pModule) override;
        virtual void HandleStart(Module* pModule) override;
        virtual void HandleStop(Module* pModule) override;
        virtual void HandleUpdate(Module* pModule) override;
    };
}

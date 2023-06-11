#pragma once
#include <BaseEditorExtension.h>
#include <GloryEditor.h>
#include <vector>
#include <imgui_internal.h>
#include <GloryContext.h>
#include <IPlayModeHandler.h>
#include <ObjectMenu.h>

extern "C" GLORY_EDITOR_EXTENSION_API Glory::Editor::BaseEditorExtension* LoadExtension();
extern "C" GLORY_EDITOR_EXTENSION_API void SetContext(Glory::GloryContext * pContext, ImGuiContext* pImGUIContext);

namespace Glory
{
    class SceneObject;
}

namespace Glory::Editor
{
    class EntityScenesEditorExtension : public BaseEditorExtension, IPlayModeHandler
    {
    public:
        EntityScenesEditorExtension();
        virtual ~EntityScenesEditorExtension();

    private:
        virtual void RegisterEditors() override;

        virtual const char* ModuleName() override;
        virtual void HandleBeforeStart(Module* pModule) override;
        virtual void HandleStart(Module* pModule) override;
        virtual void HandleStop(Module* pModule) override;
        virtual void HandleUpdate(Module* pModule) override;

        static void ConvertToPrefab(Object* pObject, const ObjectMenuType&);

    private:
        static const std::vector<std::type_index> m_ComponentsToUpdateInEditor;
    };
}

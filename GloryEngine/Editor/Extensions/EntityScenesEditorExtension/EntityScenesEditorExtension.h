#pragma once
#include <BaseEditorExtension.h>
#include <GloryEditor.h>
#include <vector>
#include <imgui_internal.h>
#include <GloryContext.h>
#include <IPlayModeHandler.h>
#include <ObjectMenu.h>

EXTENSION_H

namespace Glory
{
    class SceneObject;
    class EntitySceneObject;
}

namespace Glory::Editor
{
    class EntityScenesEditorExtension : public BaseEditorExtension, IPlayModeHandler
    {
    public:
        EntityScenesEditorExtension();
        virtual ~EntityScenesEditorExtension();

    private:
        virtual void Initialize() override;

        virtual const char* ModuleName() override;
        virtual void HandleBeforeStart(Module* pModule) override;
        virtual void HandleStart(Module* pModule) override;
        virtual void HandleStop(Module* pModule) override;
        virtual void HandleUpdate(Module* pModule) override;

        static void ConvertToPrefabMenuItem(Object* pObject, const ObjectMenuType&);
        static void ConvertToPrefab(EntitySceneObject* pObject, std::filesystem::path path);
        static void UnpackPrefabMenuItem(Object* pObject, const ObjectMenuType&);

    private:
        static const std::vector<std::type_index> m_ComponentsToUpdateInEditor;
    };
}

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
}

namespace Glory::Editor
{
    class EntityScenesEditorExtension : public BaseEditorExtension
    {
    public:
        EntityScenesEditorExtension();
        virtual ~EntityScenesEditorExtension();

    private:
        virtual void Initialize() override;

        static void ConvertToPrefabMenuItem(Object* pObject, const ObjectMenuType&);
        static void ConvertToPrefab(SceneObject* pObject, std::filesystem::path path);
        static void UnpackPrefabMenuItem(Object* pObject, const ObjectMenuType&);

    private:
        static const std::vector<std::type_index> m_ComponentsToUpdateInEditor;
    };
}

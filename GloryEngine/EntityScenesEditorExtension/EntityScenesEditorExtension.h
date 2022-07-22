#pragma once
#include <BaseEditorExtension.h>
#include <Glory.h>
#include <vector>
#include <EditorContext.h>

extern "C" GLORY_API void LoadExtension(Glory::Editor::EditorContext* pContext, std::vector<Glory::Editor::BaseEditorExtension*>& pExtensions);

namespace Glory::Editor
{
    class EntityScenesEditorExtension : public BaseEditorExtension
    {
    public:
        EntityScenesEditorExtension();
        virtual ~EntityScenesEditorExtension();

    private:
        virtual void RegisterEditors() override;
    };
}

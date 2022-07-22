#pragma once
#include <BaseEditorExtension.h>

#include <Glory.h>
#include <vector>

extern "C" GLORY_API void LoadExtension(std::vector<Glory::Editor::BaseEditorExtension*>& pExtensions);

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

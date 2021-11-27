#pragma once
#include <BaseEditorExtension.h>

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

#pragma once
#include <BaseEditorExtension.h>

#include <Reflection.h>

EXTENSION_H

namespace Glory::Editor
{
    struct UIElementType
    {
        REFLECTABLE(UIElementType,
            (uint64_t)(m_EntityID)
        );
    };

    class UIEditorExtension : public BaseEditorExtension
    {
    public:
        UIEditorExtension();
        virtual ~UIEditorExtension();

    private:
        virtual void Initialize() override;
    };
}

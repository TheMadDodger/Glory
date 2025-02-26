#pragma once
#include <BaseEditorExtension.h>

EXTENSION_H

namespace Glory::Editor
{
    class UIEditorExtension : public BaseEditorExtension
    {
    public:
        UIEditorExtension();
        virtual ~UIEditorExtension();

    private:
        virtual void Initialize() override;
    };
}

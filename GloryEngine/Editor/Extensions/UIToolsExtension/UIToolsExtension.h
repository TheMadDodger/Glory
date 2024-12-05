#pragma once
#include <BaseEditorExtension.h>

EXTENSION_H

namespace Glory::Editor
{
    class UIToolsExtension : public BaseEditorExtension
    {
    public:
        UIToolsExtension();
        virtual ~UIToolsExtension();

    private:
        virtual void Initialize() override;
    };
}

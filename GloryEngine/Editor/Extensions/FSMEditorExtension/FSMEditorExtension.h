#pragma once
#include <BaseEditorExtension.h>

EXTENSION_H

namespace Glory::Editor
{
    class FSMEditorExtension : public BaseEditorExtension
    {
    public:
        FSMEditorExtension();
        virtual ~FSMEditorExtension();

    private:
        virtual void Initialize() override;
    };
}

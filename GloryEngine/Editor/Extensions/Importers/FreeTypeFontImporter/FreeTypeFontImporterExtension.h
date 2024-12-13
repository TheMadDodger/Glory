#pragma once
#include <BaseEditorExtension.h>

EXTENSION_H

namespace Glory::Editor
{
    class FreeTypeFontImporter : public BaseEditorExtension
    {
    public:
        FreeTypeFontImporter();
        virtual ~FreeTypeFontImporter();

    private:
        virtual void Initialize() override;
    };
}

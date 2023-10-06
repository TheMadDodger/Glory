#pragma once
#include <BaseEditorExtension.h>

EXTENSION_H

namespace Glory::Editor
{
    class SDLImageImporterExtension : public BaseEditorExtension
    {
    public:
        SDLImageImporterExtension();
        virtual ~SDLImageImporterExtension();

    private:
        virtual void Initialize() override;
    };
}

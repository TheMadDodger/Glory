#pragma once
#include <BaseEditorExtension.h>

EXTENSION_H

namespace Glory::Editor
{
    class STBImageImporterExtension : public BaseEditorExtension
    {
    public:
        STBImageImporterExtension();
        virtual ~STBImageImporterExtension();

    private:
        virtual void Initialize() override;
    };
}

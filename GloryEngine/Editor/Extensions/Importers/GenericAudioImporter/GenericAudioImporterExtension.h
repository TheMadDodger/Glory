#pragma once
#include <BaseEditorExtension.h>

EXTENSION_H

namespace Glory::Editor
{
    class GenericAudioImporterExtension : public BaseEditorExtension
    {
    public:
        GenericAudioImporterExtension();
        virtual ~GenericAudioImporterExtension();

    private:
        virtual void Initialize() override;
    };
}

#pragma once
#include <BaseEditorExtension.h>

EXTENSION_H

namespace Glory::Editor
{
    class SDLAudioExtension : public BaseEditorExtension
    {
    public:
        SDLAudioExtension();
        virtual ~SDLAudioExtension();

    private:
        virtual void Initialize() override;
    };
}

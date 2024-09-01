#pragma once
#include <EntityComponentEditor.h>
#include <AudioComponents.h>

namespace Glory::Editor
{
    class AudioSourceEditor : public EntityComponentEditor<AudioSourceEditor, AudioSource>
    {
    public:
        AudioSourceEditor();
        virtual ~AudioSourceEditor();

        virtual void Initialize() override;

    private:
        virtual bool OnGUI() override;
        virtual std::string Name() override;
    };
}
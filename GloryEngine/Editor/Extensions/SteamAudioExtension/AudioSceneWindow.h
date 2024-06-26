#pragma once
#include <EditorWindow.h>

namespace Glory::Editor
{
    class AudioSceneWindow : public EditorWindowTemplate<AudioSceneWindow>
    {
    public:
        AudioSceneWindow();
        virtual ~AudioSceneWindow();

    private:
        virtual void OnGUI() override;
    };
}

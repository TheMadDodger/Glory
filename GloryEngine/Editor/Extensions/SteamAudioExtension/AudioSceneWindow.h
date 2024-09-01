#pragma once
#include <EditorWindow.h>

namespace Glory
{
    class AudioScene;
}

namespace Glory::Editor
{
    class AudioSceneWindow : public EditorWindowTemplate<AudioSceneWindow>
    {
    public:
        AudioSceneWindow();
        virtual ~AudioSceneWindow();

    private:
        virtual void OnGUI() override;
        virtual void OnOpen() override;
        virtual void OnClose() override;

        void LoadAllCachedScenes(EditorSceneManager& sceneManager);
        void ReloadSceneCache(EditorSceneManager& sceneManager, UUID sceneID);

        UUID m_SceneEventsListener;
        std::vector<AudioScene> m_AudioScenes;
    };
}

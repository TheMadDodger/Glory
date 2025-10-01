#pragma once
#include "EditorWindow.h"
namespace Glory::Editor
{
    class GameWindow : public EditorWindowTemplate<GameWindow>
    {
    public:
        GameWindow();
        virtual ~GameWindow();

    private:
        virtual void OnGUI() override;
        virtual void OnOpen() override;

    private:
        void MenuBar();
        void View();

    private:
        int m_CurrentOutputCameraIndex;
        int m_SelectedRenderTextureIndex;
    };
}
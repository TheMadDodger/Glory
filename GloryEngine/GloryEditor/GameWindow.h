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
        virtual void OnPaint() override;
        virtual void OnGUI() override;

    private:
        void MenuBar();
        void View();

    private:
        size_t m_DisplayIndex;
    };
}
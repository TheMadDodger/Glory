#pragma once
#include "EditorWindow.h"
#include <CameraRef.h>

namespace Glory::Editor
{
    class SceneWindow : public EditorWindowTemplate<SceneWindow>
    {
    public:
        SceneWindow();
        virtual ~SceneWindow();

        virtual void OnOpen() override;
        virtual void OnClose() override;

    private:
        virtual void OnPaint() override;
        virtual void OnGUI() override;
        virtual void GameThreadPaint() override;

    private:
        CameraRef m_SceneCamera;
    };
}
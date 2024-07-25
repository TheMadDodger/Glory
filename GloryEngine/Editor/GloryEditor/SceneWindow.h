#pragma once
#include "EditorWindow.h"
#include "SceneViewCamera.h"

#include <ImGuizmo.h>

namespace Glory::Editor
{
    struct ViewEvent
    {
        bool Ortho;
    };
    template<typename T>
    struct Dispatcher;

    class SceneWindow : public EditorWindowTemplate<SceneWindow>
    {
    public:
        SceneWindow();
        virtual ~SceneWindow();

        virtual void OnOpen() override;
        virtual void OnClose() override;

        static Dispatcher<ViewEvent>& GetViewEventDispatcher();

    private:
        virtual void OnGUI() override;
        virtual void Draw() override;

        void MenuBar();
        void CameraUpdate();
        void DrawScene();
        void Picking(const ImVec2& min, const ImVec2& size);

    private:
        SceneViewCamera m_SceneCamera;
        bool m_DrawGrid;
        int m_SelectedRenderTextureIndex;
        size_t m_SelectedFrameBufferIndex;
        UUID m_ViewEventID;
    };
}
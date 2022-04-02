#pragma once
#include "EditorWindow.h"
#include "SceneViewCamera.h"
#include <ImGuizmo.h>

namespace Glory::Editor
{
    class SceneWindow : public EditorWindowTemplate<SceneWindow>
    {
    public:
        SceneWindow();
        virtual ~SceneWindow();

        virtual void OnOpen() override;
        virtual void OnClose() override;

        static void EnableOrthographicView(bool enable);
        static bool IsOrthographicEnabled();

    private:
        virtual void OnGUI() override;
        virtual void Draw() override;

        void MenuBar(RenderTexture* pRenderTexture);
        void CameraUpdate();
        void DrawScene(RenderTexture* pRenderTexture);
        void Picking(const ImVec2& min, const ImVec2& size);

    private:
        SceneViewCamera m_SceneCamera;
        static bool m_Orthographic;
        bool m_DrawGrid;
        size_t m_SelectedFrameBufferIndex;
    };
}
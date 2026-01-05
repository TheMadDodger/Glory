#pragma once
#include "EditorWindow.h"
#include "SceneViewCamera.h"

#include <SceneObjectRef.h>

#include <ImGuizmo.h>

namespace Glory
{
    class GScene;
}

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

        void HandleDragAndDrop(std::string& path);
        const glm::vec3 GetPosition() const;
        const glm::quat GetRotation() const;

    private:
        SceneViewCamera m_SceneCamera;
        bool m_DrawGrid;
        int m_SelectedRenderTextureIndex;
        UUID m_ViewEventID;
        UUID m_PreviewPrefabID = 0;
        UUID m_PrefabInstance = 0;
        GScene* m_pPreviewScene;
        glm::uvec2 m_PickPos;
        bool m_BlockNextPick{ false };

        struct
        {
            SceneObjectRef m_Object;
            glm::vec3 m_WorldPosition;
            glm::vec3 m_Normal;
        } m_CurrentPick;
    };
}
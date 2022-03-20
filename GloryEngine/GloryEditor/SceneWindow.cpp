#include "SceneWindow.h"
#include "SceneViewCamera.h"
#include "EditorSceneManager.h"
#include "EditorApplication.h"
#include "EditorRenderImpl.h"
#include "Gizmos.h"
#include <CameraManager.h>
#include <Game.h>
#include <Engine.h>
#include <RendererModule.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui_internal.h>

namespace Glory::Editor
{
	bool SceneWindow::m_Orthographic = false;

	SceneWindow::SceneWindow()
		: EditorWindowTemplate("Scene", 1280.0f, 720.0f), m_DrawGrid(true)
	{
		m_WindowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar;
	}

	SceneWindow::~SceneWindow()
	{
	}

	void SceneWindow::OnOpen()
	{
		ImGuiIO& io = ImGui::GetIO();

		m_SceneCamera.Initialize();
		m_SceneCamera.m_Camera.SetResolution((int)m_WindowDimensions.x, (int)m_WindowDimensions.y);
		if (m_Orthographic) m_SceneCamera.SetOrthographic(m_WindowDimensions.x, m_WindowDimensions.y, 0.1f, 3000.0f);
		else m_SceneCamera.SetPerspective(m_WindowDimensions.x, m_WindowDimensions.y, 60.0f, 0.1f, 3000.0f);
		m_SceneCamera.m_Camera.EnableOutput(true);
	}

	void SceneWindow::OnClose()
	{
		Gizmos::Clear();
		m_SceneCamera.Cleanup();
	}

	void SceneWindow::EnableOrthographicView(bool enable)
	{
		m_Orthographic = enable;
	}

	bool SceneWindow::IsOrthographicEnabled()
	{
		return m_Orthographic;
	}

	void SceneWindow::OnGUI()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Test"))
			{
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Grid", NULL, m_DrawGrid))
			{
				m_DrawGrid = !m_DrawGrid;
			}
			ImGui::EndMenuBar();
		}

		if (ImGui::IsWindowFocused()) m_SceneCamera.Update();

		m_SceneCamera.m_Camera.SetResolution((int)m_WindowDimensions.x, (int)m_WindowDimensions.y);
		if (m_Orthographic) m_SceneCamera.SetOrthographic(m_WindowDimensions.x, m_WindowDimensions.y, 0.1f, 3000.0f);
		else m_SceneCamera.SetPerspective(m_WindowDimensions.x, m_WindowDimensions.y, 60.0f, 0.1f, 3000.0f);
		
		RenderTexture* pSceneTexture = m_SceneCamera.m_Camera.GetOutputTexture();
		if (pSceneTexture == nullptr) return;
		Texture* pTexture = pSceneTexture->GetTextureAttachment(0);
		
		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform()->GetRenderImpl();
		float aspect = m_WindowDimensions.x / m_WindowDimensions.y;
		
		ImVec2 pos = ImGui::GetCursorScreenPos();
		float width = ImGui::GetWindowWidth();
		float height = width / aspect;
		ImGui::GetWindowDrawList()->AddImage(
			pRenderImpl->GetTextureID(pTexture), ImVec2(pos.x, pos.y),
			ImVec2(pos.x + width, pos.y + height), ImVec2(0, 1), ImVec2(1, 0));

		ImGuizmo::SetDrawlist();
		float windowWidth = (float)ImGui::GetWindowWidth();
		float windowHeight = (float)ImGui::GetWindowHeight();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

		ImGuiIO& io = ImGui::GetIO();
		float viewManipulateRight = io.DisplaySize.x;
		float viewManipulateTop = 0;

		viewManipulateRight = ImGui::GetWindowPos().x + windowWidth;
		viewManipulateTop = ImGui::GetWindowPos().y;
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		m_WindowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar | (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(window->InnerRect.Min, window->InnerRect.Max) ? ImGuiWindowFlags_NoMove : 0);

		const glm::mat4& cameraView = m_SceneCamera.m_Camera.GetFinalView();
		const glm::mat4& cameraProjection = m_SceneCamera.m_Camera.GetProjection();

		glm::mat4 identityMatrix = glm::identity<glm::mat4>();
		if (m_DrawGrid) ImGuizmo::DrawGrid((float*)&cameraView, (float*)&cameraProjection, (float*)&identityMatrix, 100.f);

		//ImGuizmo::DrawCubes((float*)&cameraView, (float*)&cameraProjection, (float*)&identityMatrix, 1);

		Gizmos::DrawGizmos(cameraView, cameraProjection);

		float camDistance = 8.f;
		ImGuizmo::ViewManipulate(m_SceneCamera.m_Camera.GetViewPointer(), camDistance, ImVec2(viewManipulateRight - 256, viewManipulateTop), ImVec2(256, 256), 0x10101010);
	}

	void SceneWindow::Draw()
	{
		Game::GetGame().GetEngine()->GetRendererModule()->Submit(m_SceneCamera.m_Camera);
	}
}
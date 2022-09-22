#include "SceneWindow.h"
#include "SceneViewCamera.h"
#include "EditorSceneManager.h"
#include "EditorApplication.h"
#include "EditorRenderImpl.h"
#include "Gizmos.h"
#include "Selection.h"
#include <CameraManager.h>
#include <Game.h>
#include <Engine.h>
#include <RendererModule.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui_internal.h>
#include <GloryContext.h>

namespace Glory::Editor
{
	bool SceneWindow::m_Orthographic = false;

	SceneWindow::SceneWindow()
		: EditorWindowTemplate("Scene", 1280.0f, 720.0f), m_DrawGrid(true), m_SelectedFrameBufferIndex(0)
	{
		m_WindowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar;
	}

	SceneWindow::~SceneWindow()
	{
	}

	void SceneWindow::OnOpen()
	{
		m_SelectedFrameBufferIndex = 0;

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
		Engine* pEngine = Game::GetGame().GetEngine();
		RenderTexture* pRenderTexture = GloryContext::GetCameraManager()->GetRenderTextureForCamera(m_SceneCamera.m_Camera, pEngine, false);

		MenuBar(pRenderTexture);
		CameraUpdate();
		DrawScene(pRenderTexture);
	}

	void SceneWindow::Draw()
	{
		Game::GetGame().GetEngine()->GetRendererModule()->Submit(m_SceneCamera.m_Camera);
	}

	void SceneWindow::MenuBar(RenderTexture* pRenderTexture)
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

			if (pRenderTexture && ImGui::BeginMenu(m_SelectedFrameBufferIndex == 0 ? "Final" : pRenderTexture->AttachmentName(m_SelectedFrameBufferIndex - 1).c_str()))
			{
				if (ImGui::MenuItem("Final", NULL, m_SelectedFrameBufferIndex == 0))
					m_SelectedFrameBufferIndex = 0;

				for (size_t i = 0; i < pRenderTexture->AttachmentCount(); i++)
				{
					const std::string& name = pRenderTexture->AttachmentName(i);
					if (ImGui::MenuItem(name.c_str(), NULL, m_SelectedFrameBufferIndex == i + 1))
						m_SelectedFrameBufferIndex = i + 1;
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}

	void SceneWindow::CameraUpdate()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(window->InnerRect.Min, window->InnerRect.Max)) m_SceneCamera.Update();
		m_SceneCamera.m_Camera.SetResolution((int)m_WindowDimensions.x, (int)m_WindowDimensions.y);
		if (m_Orthographic) m_SceneCamera.SetOrthographic(m_WindowDimensions.x, m_WindowDimensions.y, 0.1f, 3000.0f);
		else m_SceneCamera.SetPerspective(m_WindowDimensions.x, m_WindowDimensions.y, 60.0f, 0.1f, 3000.0f);
	}

	void SceneWindow::DrawScene(RenderTexture* pRenderTexture)
	{
		RenderTexture* pSceneTexture = m_SceneCamera.m_Camera.GetOutputTexture();
		if (pSceneTexture == nullptr) return;

		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform()->GetRenderImpl();

		ImVec2 screenPos = ImGui::GetCursorScreenPos();
		ImVec2 regionAvail = ImGui::GetContentRegionAvail();
		float aspect = (regionAvail.x) / (regionAvail.y);
		float width = regionAvail.x;
		float height = width / aspect;

		Texture* pTexture = m_SelectedFrameBufferIndex == 0 ? pSceneTexture->GetTextureAttachment(0) : pRenderTexture->GetTextureAttachment(m_SelectedFrameBufferIndex - 1);

		ImVec2 viewportSize = ImVec2(width, height);
		ImGui::Image(pRenderImpl->GetTextureID(pTexture), viewportSize, ImVec2(0, 1), ImVec2(1, 0));

		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(screenPos.x, screenPos.y, width, height);

		ImGuiIO& io = ImGui::GetIO();
		float viewManipulateRight = io.DisplaySize.x;
		float viewManipulateTop = 0;
		Picking(screenPos, viewportSize);

		viewManipulateRight = ImGui::GetWindowPos().x + width;
		viewManipulateTop = ImGui::GetWindowPos().y;
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		m_WindowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar | (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(window->InnerRect.Min, window->InnerRect.Max) ? ImGuiWindowFlags_NoMove : 0);

		const glm::mat4& cameraView = m_SceneCamera.m_Camera.GetView();
		const glm::mat4& cameraProjection = m_SceneCamera.m_Camera.GetProjection();

		glm::mat4 identityMatrix = glm::identity<glm::mat4>();
		if (m_DrawGrid) ImGuizmo::DrawGrid((float*)&cameraView, (float*)&cameraProjection, (float*)&identityMatrix, 100.f);

		//ImGuizmo::DrawCubes((float*)&cameraView, (float*)&cameraProjection, (float*)&identityMatrix, 1);

		Gizmos::DrawGizmos(cameraView, cameraProjection);

		float camDistance = 8.f;
		ImGuizmo::ViewManipulate(m_SceneCamera.m_Camera.GetViewPointer(), camDistance, ImVec2(viewManipulateRight - 256, viewManipulateTop + 64), ImVec2(256, 256), 0x10101010);
	}

	void SceneWindow::Picking(const ImVec2& min, const ImVec2& size)
	{
		ImVec2 viewportMax = ImVec2(min.x + size.x, min.y + size.y);
		ImVec2 coord = ImGui::GetMousePos();
		glm::vec2 viewportCoord = glm::vec2(coord.x - min.x, coord.y - min.y);
		viewportCoord = viewportCoord / glm::vec2(size.x, size.y);

		glm::uvec2 resolution = m_SceneCamera.m_Camera.GetResolution();
		glm::uvec2 textureCoord = viewportCoord * (glm::vec2)resolution;
		textureCoord.y = resolution.y - textureCoord.y;

		Game::GetGame().GetEngine()->GetRendererModule()->SetNextFramePick(textureCoord, m_SceneCamera.m_Camera);

		if (!ImGuizmo::IsOver() && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && ImGui::IsMouseHoveringRect(min, viewportMax))
		{
			Engine* pEngine = Game::GetGame().GetEngine();
			Selection::SetActiveObject(pEngine->GetScenesModule()->GetHoveringObject());
		}
	}
}
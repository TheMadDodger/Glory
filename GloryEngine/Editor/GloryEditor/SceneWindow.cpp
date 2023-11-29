#include "SceneWindow.h"
#include "SceneViewCamera.h"
#include "EditorSceneManager.h"
#include "EditorApplication.h"
#include "EditorRenderImpl.h"
#include "Gizmos.h"
#include "Selection.h"
#include "EditableEntity.h"
#include "EntityEditor.h"

#include <CameraManager.h>
#include <SceneManager.h>
#include <Engine.h>
#include <RendererModule.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui_internal.h>
#include <GloryContext.h>
#include <EditorUI.h>
#include <Shortcuts.h>
#include <Dispatcher.h>

namespace Glory::Editor
{
	static const char* Shortcut_View_Perspective = "Switch To Perspective";
	static const char* Shortcut_View_Orthographic = "Switch To Orthographic";

	SceneWindow::SceneWindow()
		: EditorWindowTemplate("Scene", 1280.0f, 720.0f),
		m_DrawGrid(true), m_SelectedFrameBufferIndex(0),
		m_ViewEventID(0)
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
		m_SceneCamera.m_Width = (uint32_t)m_WindowDimensions.x;
		m_SceneCamera.m_Height = (uint32_t)m_WindowDimensions.x;
		m_SceneCamera.SetPerspective((uint32_t)m_WindowDimensions.x, (uint32_t)m_WindowDimensions.y, 60.0f, 0.1f, 3000.0f);
		m_SceneCamera.m_Camera.EnableOutput(true);

		m_ViewEventID = GetViewEventDispatcher().AddListener([&](const ViewEvent& e) {
			m_SceneCamera.m_IsOrthographic = e.Ortho;
			m_SceneCamera.UpdateCamera();
		});
	}

	void SceneWindow::OnClose()
	{
		Gizmos::Clear();
		m_SceneCamera.Cleanup();

		GetViewEventDispatcher().RemoveListener(m_ViewEventID);
	}

	Dispatcher<ViewEvent>& SceneWindow::GetViewEventDispatcher()
	{
		static Dispatcher<ViewEvent> dispatcher;
		return dispatcher;
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
		Game::GetGame().GetEngine()->GetMainModule<RendererModule>()->Submit(m_SceneCamera.m_Camera);
	}

	void SceneWindow::MenuBar(RenderTexture* pRenderTexture)
	{
		if (ImGui::BeginMenuBar())
		{
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

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Perspective", Shortcuts::GetShortcutString(Shortcut_View_Perspective).data(), !m_SceneCamera.m_IsOrthographic))
				{
					m_SceneCamera.m_IsOrthographic = false;
					m_SceneCamera.UpdateCamera();
				}
				if (ImGui::MenuItem("Orthographic", Shortcuts::GetShortcutString(Shortcut_View_Orthographic).data(), m_SceneCamera.m_IsOrthographic))
				{
					m_SceneCamera.m_IsOrthographic = true;
					m_SceneCamera.UpdateCamera();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Camera"))
			{
				ImGui::TextUnformatted("				Camera Settings					");
				ImGui::Separator();
				bool change = false;
				change |= EditorUI::InputFloat("FOV", &m_SceneCamera.m_HalfFOV, 0, 175.0f);
				change |= EditorUI::InputFloat("Near", &m_SceneCamera.m_Near, 0.001f);
				change |= EditorUI::InputFloat("Far", &m_SceneCamera.m_Far, m_SceneCamera.m_Near + 1.0f);
				EditorUI::InputFloat("Fly Speed", &m_SceneCamera.m_MovementSpeed, 0.001f);
				EditorUI::InputFloat("Sensitivity", &m_SceneCamera.m_FreeLookSensitivity, 0.001f);
				EditorUI::InputFloat("Zoom Sensitivity", &m_SceneCamera.m_ZoomSensitivity, 0.001f);
				if (change) m_SceneCamera.UpdateCamera();
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

	void SceneWindow::CameraUpdate()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(window->InnerRect.Min, window->InnerRect.Max)) m_SceneCamera.Update();
		m_SceneCamera.m_Width = (uint32_t)m_WindowDimensions.x;
		m_SceneCamera.m_Height = (uint32_t)m_WindowDimensions.x;
		if (!m_SceneCamera.SetResolution((uint32_t)m_WindowDimensions.x, (uint32_t)m_WindowDimensions.y)) return;
		m_SceneCamera.UpdateCamera();
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

		Game::GetGame().GetEngine()->GetMainModule<RendererModule>()->SetNextFramePick(textureCoord, m_SceneCamera.m_Camera);

		if (!ImGuizmo::IsOver() && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && ImGui::IsMouseHoveringRect(min, viewportMax))
		{
			Engine* pEngine = Game::GetGame().GetEngine();
			GScene* pScene = pEngine->GetSceneManager()->GetHoveringEntityScene();
			if (!pScene)
			{
				Selection::SetActiveObject(nullptr);
				return;
			}
			const UUID objectID = pEngine->GetSceneManager()->GetHoveringEntityUUID();
			Entity entityHandle = pScene->GetEntityByUUID(objectID);
			if (!entityHandle.IsValid())
			{
				Selection::SetActiveObject(nullptr);
				return;
			}
			Selection::SetActiveObject(GetEditableEntity(entityHandle.GetEntityID(), pScene));
		}
	}
}

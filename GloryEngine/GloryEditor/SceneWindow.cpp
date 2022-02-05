#include "SceneWindow.h"
#include "SceneViewCamera.h"
#include "EditorSceneManager.h"
#include "EditorApplication.h"
#include "EditorRenderImpl.h"
#include <CameraManager.h>
#include <Game.h>
#include <Engine.h>
#include <RendererModule.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Glory::Editor
{
	SceneWindow::SceneWindow() : EditorWindowTemplate("Scene", 1280.0f, 720.0f) {}

	SceneWindow::~SceneWindow()
	{
	}

	void SceneWindow::OnOpen()
	{
		ImGuiIO& io = ImGui::GetIO();

		m_SceneCamera.Initialize();
		m_SceneCamera.m_Camera.SetResolution((int)m_WindowDimensions.x, (int)m_WindowDimensions.y);
		m_SceneCamera.m_Camera.SetPerspectiveProjection((int)m_WindowDimensions.x, (int)m_WindowDimensions.y, 60.0f, 0.1f, 3000.0f);
		m_SceneCamera.m_Camera.EnableOutput(true);
	}

	void SceneWindow::OnClose()
	{
		m_SceneCamera.Cleanup();
	}

	void SceneWindow::OnGUI()
	{
		if (ImGui::IsWindowFocused()) m_SceneCamera.Update();

		m_SceneCamera.m_Camera.SetResolution((int)m_WindowDimensions.x, (int)m_WindowDimensions.y);
		m_SceneCamera.m_Camera.SetPerspectiveProjection((int)m_WindowDimensions.x, (int)m_WindowDimensions.y, 60.0f, 0.1f, 3000.0f);
		
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
	}

	void SceneWindow::Draw()
	{
		Game::GetGame().GetEngine()->GetRendererModule()->Submit(m_SceneCamera.m_Camera);
	}
}
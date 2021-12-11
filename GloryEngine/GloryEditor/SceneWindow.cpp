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
	SceneWindow::SceneWindow() : EditorWindowTemplate("Scene", 1280.0f, 720.0f)
	{
		//m_pRenderTexture = RenderTexture::CreateRenderTexture(1280, 720, true);
	}

	SceneWindow::~SceneWindow()
	{
	}

	void SceneWindow::OnOpen()
	{
		m_SceneCamera = CameraManager::GetNewOrUnusedCamera();
		m_SceneCamera.SetResolution(m_WindowDimensions.x, m_WindowDimensions.y);
		m_SceneCamera.SetPerspectiveProjection(m_WindowDimensions.x, m_WindowDimensions.y, 60.0f, 0.1f, 3000.0f);
		glm::mat4 matrix = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		m_SceneCamera.SetView(matrix);
	}

	void SceneWindow::OnClose()
	{
		m_SceneCamera.Free();
	}

	void SceneWindow::OnPaint()
	{
		//m_SceneCamera.Update();

		//m_pRenderTexture->Use();
		//BaseGame::GetGame()->GetGameContext().pRenderer->ClearBackground();
		//GameScene* pCurrentScene = EditorSceneManager::GetActiveScene();
		//if (pCurrentScene == nullptr) return;
		//
		//CameraComponent* pPreviousCamera = pCurrentScene->GetActiveCamera();
		//pCurrentScene->SetActiveCamera(SceneViewCamera::GetSceneCamera()->GetCameraComponent());
		//EditorApp::GetEditorApp()->RenderScene();
		//pCurrentScene->SetActiveCamera(pPreviousCamera);
		//m_pRenderTexture->StopUse();
	}

	void SceneWindow::OnGUI()
	{
		//if (ImGui::IsWindowFocused()) EditorApp::GetEditorApp()->UpdateGameObject(SceneViewCamera::GetSceneCamera());

		m_SceneCamera.SetResolution(m_WindowDimensions.x, m_WindowDimensions.y);
		m_SceneCamera.SetPerspectiveProjection(m_WindowDimensions.x, m_WindowDimensions.y, 60.0f, 0.1f, 3000.0f);
		
		RenderTexture* pSceneTexture = m_SceneCamera.GetRenderTexture();
		if (pSceneTexture == nullptr) return;
		Texture* pTexture = pSceneTexture->GetTexture();
		
		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform()->GetRenderImpl();
		float aspect = m_WindowDimensions.x / m_WindowDimensions.y;
		
		ImVec2 pos = ImGui::GetCursorScreenPos();
		float width = ImGui::GetWindowWidth();
		float height = width / aspect;
		ImGui::GetWindowDrawList()->AddImage(
			pRenderImpl->GetTextureID(pTexture), ImVec2(pos.x, pos.y),
			ImVec2(pos.x + width, pos.y + height), ImVec2(0, 1), ImVec2(1, 0));
	}

	void SceneWindow::GameThreadPaint()
	{
		Game::GetGame().GetEngine()->GetRendererModule()->Submit(m_SceneCamera);
	}
}
#include "stdafx.h"
#include "SceneWindow.h"
//#include "EditorApp.h"
//#include <GameScene.h>
#include "SceneViewCamera.h"
#include "EditorSceneManager.h"

namespace Glory::Editor
{
	SceneWindow::SceneWindow() : EditorWindowTemplate("Scene", 1280.0f, 720.0f)
	{
		//m_pRenderTexture = RenderTexture::CreateRenderTexture(1280, 720, true);
	}

	SceneWindow::~SceneWindow()
	{
	}

	void SceneWindow::OnPaint()
	{
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
		//
		//GLsizei w = (GLsizei)m_WindowDimensions.x;
		//GLsizei h = (GLsizei)m_WindowDimensions.y;
		//float aspect = (float)w / (float)h;
		//
		//ImVec2 pos = ImGui::GetCursorScreenPos();
		//float width = ImGui::GetWindowWidth();
		//float height = width / aspect;
		//ImGui::GetWindowDrawList()->AddImage(
		//	(void*)m_pRenderTexture->GetTextureID(), ImVec2(pos.x, pos.y),
		//	ImVec2(pos.x + width, pos.y + height), ImVec2(0, 1), ImVec2(1, 0));
	}
}
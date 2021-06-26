#include "stdafx.h"
#include "GameWindow.h"
//#include "EditorApp.h"
//#include <RenderTexture.h>

namespace Glory::Editor
{
	GameWindow::GameWindow() : EditorWindowTemplate("Game", 1280.0f, 720.0f)
	{
	}

	GameWindow::~GameWindow()
	{
	}

	void GameWindow::OnPaint()
	{
		//BaseGame::GetGame()->GetGameContext().pRenderer->ClearBackground();
		//EditorApp::GetEditorApp()->RenderScene();
	}

	void GameWindow::OnGUI()
	{
		//GLsizei w = (GLsizei)m_WindowDimensions.x;
		//GLsizei h = (GLsizei)m_WindowDimensions.y;
		//float aspect = (float)w / (float)h;
		//
		//ImVec2 pos = ImGui::GetCursorScreenPos();
		//float width = ImGui::GetWindowWidth();
		//float height = width / aspect;
		//ImGui::GetWindowDrawList()->AddImage(
		//	(void*)RenderTexture::GetDefaultRenderTexture()->GetTextureID(), ImVec2(pos.x, pos.y),
		//	ImVec2(pos.x + width, pos.y + height), ImVec2(0, 1), ImVec2(1, 0));
	}
}
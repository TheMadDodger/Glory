#include <DisplayManager.h>
#include <imgui.h>
#include "GameWindow.h"
#include "EditorApplication.h"
#include "ImGuiHelpers.h"

namespace Glory::Editor
{
	GameWindow::GameWindow() : EditorWindowTemplate("Game", 1280.0f, 720.0f), m_DisplayIndex(0)
	{
		m_WindowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar;
	}

	GameWindow::~GameWindow()
	{
	}

	void GameWindow::OnGUI()
	{
		MenuBar();
		View();
	}

	void GameWindow::MenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			std::string selectedName = "Display " + std::to_string(m_DisplayIndex);
			if (ImGui::BeginMenu(selectedName.data()))
			{
				for (size_t i = 0; i < DisplayManager::MAX_DISPLAYS; i++)
				{
					std::string name = "Display " + std::to_string(i);
					bool selected = m_DisplayIndex == i;
					if (ImGui::MenuItem(name.data(), NULL, selected))
					{
						m_DisplayIndex = i;
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}

	void GameWindow::View()
	{
		RenderTexture* pDisplayTexture = EditorApplication::GetInstance()->GetEngine()->GetDisplayManager().GetDisplayRenderTexture(m_DisplayIndex);
		if (pDisplayTexture == nullptr) return;
		Texture* pTexture = pDisplayTexture->GetTextureAttachment(0);

		uint32_t width, height;
		pDisplayTexture->GetDimensions(width, height);
		float textureAspect = (float)width / (float)height;

		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		pos = pos + vMin;
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();
		vMax = vMax - vMin;
		float maxWidth = vMax.x;
		float maxHeight = vMax.y;

		float actualHeight = maxWidth / textureAspect;

		if (actualHeight > maxHeight)
		{
			float diff = actualHeight - maxHeight;
			float widthOffset = diff * textureAspect;
			maxWidth -= widthOffset;
			actualHeight = maxWidth / textureAspect;
		}

		ImVec2 halfMax = vMax / 2.0f;
		ImVec2 center = pos + halfMax;

		ImVec2 halfOffsets = ImVec2(maxWidth / 2.0f, actualHeight / 2.0f);

		//ImGui::GetForegroundDrawList()->AddRect(center, center + halfMax, IM_COL32(255, 255, 0, 255));
		//ImGui::GetForegroundDrawList()->AddRect(center, center - halfMax, IM_COL32(255, 255, 0, 255));

		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform().GetRenderImpl();

		ImGui::GetWindowDrawList()->AddImage(
			pRenderImpl->GetTextureID(pTexture), center - halfOffsets,
			center + halfOffsets, ImVec2(0, 1), ImVec2(1, 0));
	}
}
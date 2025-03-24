#include "GameWindow.h"
#include "EditorApplication.h"
#include "ImGuiHelpers.h"
#include "EditorUI.h"

#include <DisplayManager.h>
#include <imgui.h>
#include <InputModule.h>

namespace Glory::Editor
{
	/* 1080p selected by default */
	size_t SelectedAspect = 1;
	size_t SelectedResolution = 3;
	glm::vec2 CustomResolution{ 1920.0f, 1080.0f };

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

	void GameWindow::OnOpen()
	{
		const glm::uvec2& resolution = Resolutions[SelectedAspect][SelectedResolution];
		EditorApplication::GetInstance()->GetEngine()->GetDisplayManager().SetResolution(resolution.x, resolution.y);
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

			const glm::uvec2& selectedResolution = Resolutions[SelectedAspect][SelectedResolution];
			std::stringstream str;
			str << selectedResolution.x << "x" << selectedResolution.y;

			DisplayManager& displays = EditorApplication::GetInstance()->GetEngine()->GetDisplayManager();

			if (ImGui::BeginMenu(str.str().data()))
			{
				for (size_t i = 0; i < AspectRatioCount; ++i)
				{
					if (ImGui::BeginMenu(AspectRatios[i]))
					{
						for (size_t j = 0; j < Resolutions[i].size(); ++j)
						{
							const glm::uvec2& resolution = Resolutions[i][j];
							str.str("");
							str << resolution.x << "x" << resolution.y;
							if (ImGui::MenuItem(str.str().data(), NULL, SelectedAspect == i && SelectedResolution == j))
							{
								SelectedAspect = i;
								SelectedResolution = j;
								CustomResolution = resolution;
								displays.SetResolution(resolution.x, resolution.y);
							}
						}
						ImGui::EndMenu();
					}
				}
				if (ImGui::BeginMenu("Custom"))
				{
					EditorUI::PushFlag(EditorUI::Flag::NoLabel);
					const bool change = EditorUI::InputFloat2("Resolution", &CustomResolution, 1, FLT_MAX, 1);
					EditorUI::PopFlag();
					if (change)
						displays.SetResolution(CustomResolution.x, CustomResolution.y);
					ImGui::EndMenu();
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

		const ImVec2 halfMax = vMax / 2.0f;
		const ImVec2 center = pos + halfMax;

		const ImVec2 halfOffsets = ImVec2(maxWidth / 2.0f, actualHeight / 2.0f);

		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform().GetRenderImpl();

		const ImVec2 topLeft = center - halfOffsets;
		const ImVec2 bottomRight = center + halfOffsets;

		if (m_IsFocused || ImGui::IsWindowHovered())
		{
			InputModule* pInput = EditorApplication::GetInstance()->GetEngine()->GetMainModule<InputModule>();
			pInput->SetCursorBounds({ topLeft.x, topLeft.y, bottomRight.x, bottomRight.y });

			const glm::vec2 size{ bottomRight.x - topLeft.x, bottomRight.y - topLeft.y };
			if (size.x && size.y)
			{
				const glm::vec2 screenScale = size / glm::vec2(float(width), float(height));
				pInput->SetScreenScale(screenScale);
			}
		}

		ImGui::GetWindowDrawList()->AddImage(
			pRenderImpl->GetTextureID(pTexture), topLeft,
			bottomRight, ImVec2(0, 1), ImVec2(1, 0));
	}
}
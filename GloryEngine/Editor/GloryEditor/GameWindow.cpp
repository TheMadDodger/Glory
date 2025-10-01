#include "GameWindow.h"
#include "EditorApplication.h"
#include "ImGuiHelpers.h"
#include "EditorUI.h"

#include <imgui.h>
#include <InputModule.h>
#include <RendererModule.h>

namespace Glory::Editor
{
	/* 1080p selected by default */
	size_t SelectedAspect = 1;
	size_t SelectedResolution = 3;
	glm::vec2 CustomResolution{ 1920.0f, 1080.0f };

	GameWindow::GameWindow(): EditorWindowTemplate("Game", 1280.0f, 720.0f),
		m_CurrentOutputCameraIndex(-1), m_SelectedRenderTextureIndex(-1)
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
		RendererModule* pRenderer = EditorApplication::GetInstance()->GetEngine()->GetMainModule<RendererModule>();
		pRenderer->OnWindowResize(resolution);
	}

	void GameWindow::MenuBar()
	{
		RendererModule* pRenderer = EditorApplication::GetInstance()->GetEngine()->GetMainModule<RendererModule>();

		if (m_SelectedRenderTextureIndex == -1)
			m_SelectedRenderTextureIndex = pRenderer->DefaultAttachmenmtIndex();

		if (ImGui::BeginMenuBar())
		{
			const glm::uvec2& selectedResolution = Resolutions[SelectedAspect][SelectedResolution];
			std::stringstream str;
			str << selectedResolution.x << "x" << selectedResolution.y;

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
								pRenderer->OnWindowResize(resolution);
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
						pRenderer->OnWindowResize(glm::uvec2(CustomResolution));
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			const size_t outputCameraCount = pRenderer->GetOutputCameraCount();
			if (m_CurrentOutputCameraIndex != -1 && m_CurrentOutputCameraIndex > outputCameraCount)
				m_CurrentOutputCameraIndex = 0;

			if (ImGui::BeginMenu(m_CurrentOutputCameraIndex == -1 ? "Final output color" : std::string("Camera " + std::to_string(m_CurrentOutputCameraIndex)).data()))
			{
				if (ImGui::MenuItem("Final output", NULL, m_CurrentOutputCameraIndex == -1))
					m_CurrentOutputCameraIndex = -1;

				for (size_t i = 0; i < outputCameraCount; ++i)
				{
					if (ImGui::MenuItem(std::string("Camera " + std::to_string(i)).data(), NULL, m_CurrentOutputCameraIndex == i))
					{
						m_CurrentOutputCameraIndex = i;
					}
				}

				ImGui::EndMenu();
			}

			ImGui::BeginDisabled(m_CurrentOutputCameraIndex == -1);
			if (ImGui::BeginMenu(pRenderer->CameraAttachmentPreviewName(m_SelectedRenderTextureIndex).data()))
			{
				for (size_t i = 0; i < pRenderer->CameraAttachmentPreviewCount(); ++i)
				{
					if (ImGui::MenuItem(pRenderer->CameraAttachmentPreviewName(i).data(), NULL, m_SelectedRenderTextureIndex == i))
					{
						m_SelectedRenderTextureIndex = i;
					}
				}

				ImGui::EndMenu();
			}
			ImGui::EndDisabled();
			ImGui::EndMenuBar();
		}
	}

	void GameWindow::View()
	{
		RendererModule* pRenderer = EditorApplication::GetInstance()->GetEngine()->GetMainModule<RendererModule>();

		const size_t outputCameraCount = pRenderer->GetOutputCameraCount();
		if (m_CurrentOutputCameraIndex != -1 && m_CurrentOutputCameraIndex > outputCameraCount)
			m_CurrentOutputCameraIndex = 0;
		if (outputCameraCount == 0)
			m_CurrentOutputCameraIndex = -1;

		TextureHandle texture = NULL;
		glm::uvec2 resolution{};

		if (m_CurrentOutputCameraIndex == -1)
		{
			texture = pRenderer->FinalColor();
			resolution = pRenderer->Resolution();
		}
		else
		{
			CameraRef camera = pRenderer->GetOutputCamera(m_CurrentOutputCameraIndex);
			texture = pRenderer->CameraAttachmentPreview(camera, size_t(m_SelectedRenderTextureIndex));
			resolution = camera.GetResolution();
		}

		const float textureAspect = (float)resolution.x/(float)resolution.y;

		const ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();
		ImVec2 pos = ImGui::GetWindowPos();
		pos = pos + vMin;
		vMax = vMax - vMin;
		float maxWidth = vMax.x;
		const float maxHeight = vMax.y;

		float actualHeight = maxWidth/textureAspect;

		if (actualHeight > maxHeight)
		{
			float diff = actualHeight - maxHeight;
			float widthOffset = diff*textureAspect;
			maxWidth -= widthOffset;
			actualHeight = maxWidth/textureAspect;
		}

		const ImVec2 halfMax = vMax/2.0f;
		const ImVec2 center = pos + halfMax;

		const ImVec2 halfOffsets = ImVec2(maxWidth/2.0f, actualHeight/2.0f);

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
				const glm::vec2 screenScale = size/glm::vec2(float(resolution.x), float(resolution.y));
				pInput->SetScreenScale(screenScale);
			}
		}

		ImGui::GetWindowDrawList()->AddImage(
			pRenderImpl->GetTextureID(texture), topLeft,
			bottomRight, ImVec2(0, 1), ImVec2(1, 0));
	}
}
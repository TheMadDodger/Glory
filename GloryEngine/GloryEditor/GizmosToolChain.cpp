#include "GizmosToolChain.h"
#include "Toolbar.h"
#include "Gizmos.h"
#include "EditorAssets.h"
#include "EditorRenderImpl.h"
#include "EditorApplication.h"
#include "ImGuiHelpers.h"
#include <ImGuizmo.h>

namespace Glory::Editor
{
	const ImGuizmo::OPERATION GizmosToolChain::OPERATIONS[] = {
		ImGuizmo::TRANSLATE,
		ImGuizmo::ROTATE,
		ImGuizmo::SCALE,
		ImGuizmo::UNIVERSAL,
	};
	const std::string GizmosToolChain::OPERATION_TEXTURES[] = {
		EditorAssetNames::GizmoMove,
		EditorAssetNames::GizmoRotate,
		EditorAssetNames::GizmoScale,
		EditorAssetNames::GizmoUniversal,
	};
	const size_t GizmosToolChain::OPERATIONS_COUNT = 4;

	const ImGuizmo::MODE GizmosToolChain::MODES[] = {
		ImGuizmo::LOCAL,
		ImGuizmo::WORLD
	};
	const std::string GizmosToolChain::MODE_TEXTURES[] = {
		EditorAssetNames::GizmoLocal,
		EditorAssetNames::GizmoWorld
	};
	const size_t GizmosToolChain::MODES_COUNT = 2;

	GizmosToolChain::GizmosToolChain()
	{
		Toolbar::AddToolChain(ToolChainPosition::Left, this);
	}

	GizmosToolChain::~GizmosToolChain()
	{
	}

	void GizmosToolChain::DrawToolchain(float& cursor, const ImVec2& maxButtonSize)
	{
		DrawOperations(cursor, maxButtonSize);
		cursor += 5.0f;
		DrawModes(cursor, maxButtonSize);
	}

	void GizmosToolChain::DrawOperations(float& cursor, const ImVec2& maxButtonSize)
	{
		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform()->GetRenderImpl();

		ImVec4 activeColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
		int styleColorCount = 0;
		for (size_t i = 0; i < OPERATIONS_COUNT; i++)
		{
			ImGuizmo::OPERATION op = OPERATIONS[i];
			bool selected = Gizmos::m_DefaultOperation == op;
			styleColorCount = 0;
			if (selected)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, activeColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
				styleColorCount = 3;
			}

			ImGui::SameLine(cursor);
			std::string label = "##OP_" + std::to_string(i);
			Texture* pTexture = EditorAssets::GetTexture(OPERATION_TEXTURES[i]);

			if (ImGui::ImageButton(pRenderImpl->GetTextureID(pTexture), maxButtonSize * 0.8f))
			{
				Gizmos::m_DefaultOperation = op;
			}
			ImGui::PopStyleColor(styleColorCount);
			cursor += maxButtonSize.x;
		}
	}

	void GizmosToolChain::DrawModes(float& cursor, const ImVec2& maxButtonSize)
	{
		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform()->GetRenderImpl();

		ImVec4 activeColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
		int styleColorCount = 0;
		for (size_t i = 0; i < MODES_COUNT; i++)
		{
			ImGuizmo::MODE mode = MODES[i];
			bool selected = Gizmos::m_DefaultMode == mode;
			styleColorCount = 0;
			if (selected)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, activeColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
				styleColorCount = 3;
			}

			ImGui::SameLine(cursor);
			std::string label = "##MODE_" + std::to_string(i);
			Texture* pTexture = EditorAssets::GetTexture(MODE_TEXTURES[i]);

			if (ImGui::ImageButton(pRenderImpl->GetTextureID(pTexture), maxButtonSize * 0.8f))
			{
				Gizmos::m_DefaultMode = mode;
			}
			ImGui::PopStyleColor(styleColorCount);
			cursor += maxButtonSize.x;
		}
	}
}

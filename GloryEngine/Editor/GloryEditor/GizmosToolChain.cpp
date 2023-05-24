#include "GizmosToolChain.h"
#include "Toolbar.h"
#include "Gizmos.h"
#include "EditorAssets.h"
#include "EditorRenderImpl.h"
#include "EditorApplication.h"
#include "ImGuiHelpers.h"
#include <ImGuizmo.h>

#include <IconsFontAwesome6.h>

namespace Glory::Editor
{
	const ImGuizmo::OPERATION OPERATIONS[] = {
		ImGuizmo::TRANSLATE,
		ImGuizmo::ROTATE,
		ImGuizmo::SCALE,
		ImGuizmo::UNIVERSAL,
	};
	const std::string_view OPERATION_ICONS[] = {
		ICON_FA_UP_DOWN_LEFT_RIGHT,
		ICON_FA_ARROWS_ROTATE,
		ICON_FA_UP_RIGHT_AND_DOWN_LEFT_FROM_CENTER,
		ICON_FA_ARROW_DOWN_UP_ACROSS_LINE,
	};

	const std::string_view OPERATION_TOOLTIPS[] = {
		"Translate gizmo",
		"Rotate gizmo",
		"Scale gizmo",
		"Universal gizmo",
	};

	const size_t OPERATIONS_COUNT = 4;

	const ImGuizmo::MODE MODES[] = {
		ImGuizmo::LOCAL,
		ImGuizmo::WORLD
	};
	const std::string_view MODE_ICONS[] = {
		ICON_FA_LOCATION_DOT,
		ICON_FA_GLOBE
	};
	const std::string_view MODE_TOOLTIPS[] = {
		"Local gizmo transform",
		"World gizmo transform"
	};

	const size_t MODES_COUNT = 2;

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

			if (ImGui::Button(OPERATION_ICONS[i].data(), maxButtonSize))
			{
				Gizmos::m_DefaultOperation = op;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(OPERATION_TOOLTIPS[i].data());
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

			if (ImGui::Button(MODE_ICONS[i].data(), maxButtonSize))
			{
				Gizmos::m_DefaultMode = mode;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(MODE_TOOLTIPS[i].data());
			ImGui::PopStyleColor(styleColorCount);
			cursor += maxButtonSize.x;
		}
	}
}

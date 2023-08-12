#include "GridToolchain.h"
#include "Toolbar.h"
#include "EditorUI.h"
#include "Shortcuts.h"

#include <IconsFontAwesome6.h>

namespace Glory::Editor
{
	const ImGuizmo::OPERATION OPERATIONS[] = {
		ImGuizmo::TRANSLATE,
		ImGuizmo::ROTATE,
		ImGuizmo::SCALE,
	};
	const std::string_view OPERATION_ICONS[] = {
		ICON_FA_BORDER_TOP_LEFT,
		ICON_FA_KIP_SIGN,
		ICON_FA_EXPAND,
	};

	const std::string_view OPERATION_TOOLTIPS_ON[] = {
		"Disable translate grid snapping",
		"Disable rotate angle snapping",
		"Disable scale grid snapping",
	};

	const std::string_view OPERATION_TOOLTIPS_OFF[] = {
		"Enable translate grid snapping",
		"Enable rotate angle snapping",
		"Enable scale grid snapping",
	};

	const std::string_view OPERATION_SLIDER_TOOLTIPS[] = {
		"Translate grid snapping value",
		"Rotate angle snapping value",
		"Scale grid snapping value",
	};

	const size_t OPERATIONS_COUNT = 3;

	GridToolchain::GridToolchain() :
		m_SnappingGrids{
			{1.0f, 1.0f, 1.0f},
			{10.0f, 10.0f, 10.0f},
			{0.1f, 0.1f, 0.1f}
		},
		m_SnappingEnabled{ true, true, true }
	{
		Toolbar::AddToolChain(ToolChainPosition::Left, this);

		Shortcuts::AddAction("Toggle translation snapping", [&]() {
			m_SnappingEnabled[0] = !m_SnappingEnabled[0];
		});

		Shortcuts::AddAction("Toggle rotation angle snapping", [&]() {
			m_SnappingEnabled[1] = !m_SnappingEnabled[1];
		});

		Shortcuts::AddAction("Toggle scale snapping", [&]() {
			m_SnappingEnabled[2] = !m_SnappingEnabled[2];
		});
	}

	const float* GridToolchain::GetSnap(ImGuizmo::OPERATION op)
	{
		switch (op)
		{
		case ImGuizmo::TRANSLATE:
			return m_SnappingEnabled[0] ? reinterpret_cast<const float*>(&m_SnappingGrids[0]) : nullptr;
		case ImGuizmo::ROTATE:
			return m_SnappingEnabled[1] ? reinterpret_cast<const float*>(&m_SnappingGrids[1]) : nullptr;
		case ImGuizmo::SCALE:
			return m_SnappingEnabled[2] ? reinterpret_cast<const float*>(&m_SnappingGrids[2]) : nullptr;
		case ImGuizmo::UNIVERSAL:
			return m_SnappingEnabled[0] ? reinterpret_cast<const float*>(&m_SnappingGrids[0])
				: m_SnappingEnabled[1] ? reinterpret_cast<const float*>(&m_SnappingGrids[1])
				: m_SnappingEnabled[2] ? reinterpret_cast<const float*>(&m_SnappingGrids[2]) : nullptr;
		}

		throw new std::exception("Reached unreachable code");
	}

	void GridToolchain::DrawToolchain(float& cursor, const ImVec2& maxButtonSize)
	{
		const ImVec4 activeColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
		EditorUI::PushFlag(EditorUI::Flag::NoLabel);

		constexpr float inputFloatWidthFactor = 1.8f;
		constexpr float padding = 5.0f;

		cursor += padding;
		for (size_t i = 0; i < OPERATIONS_COUNT; ++i)
		{
			ImGui::PushID(i);
			ImGui::SameLine(cursor);

			int styleColorCount = 0;
			if (m_SnappingEnabled[i])
			{
				ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
				styleColorCount = 2;
			}

			const std::string_view tooltip = m_SnappingEnabled[i] ? OPERATION_TOOLTIPS_ON[i] : OPERATION_TOOLTIPS_OFF[i];

			if (ImGui::Button(OPERATION_ICONS[i].data(), { maxButtonSize.x, 24.0f }))
				m_SnappingEnabled[i] = !m_SnappingEnabled[i];

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(tooltip.data());
			ImGui::PopStyleColor(styleColorCount);

			cursor += maxButtonSize.x;
			ImGui::SameLine(cursor);
			ImGui::PushItemWidth(maxButtonSize.x*inputFloatWidthFactor);
			if (EditorUI::InputFloat("snap", reinterpret_cast<float*>(&m_SnappingGrids[i]), 0.0f, FLT_MAX, 0.1f))
				m_SnappingGrids[i].y = m_SnappingGrids[i].z = m_SnappingGrids[i].x;
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(OPERATION_SLIDER_TOOLTIPS[i].data());
			ImGui::PopItemWidth();
			cursor += maxButtonSize.x*inputFloatWidthFactor + padding;
			ImGui::PopID();
		}

		cursor -= padding;

		EditorUI::PopFlag();
	}
}

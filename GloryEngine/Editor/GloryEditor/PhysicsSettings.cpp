#include "ProjectSettings.h"

#include <LayerManager.h>

#include <imgui.h>

namespace Glory::Editor
{
	bool PhysicsSettings::OnGui()
	{
		ImGui::BeginChild("Input Settings");

		YAML::Node collisionMatrixNode = m_SettingsNode["CollisionMatrix"];
		if (!collisionMatrixNode.IsSequence())
			collisionMatrixNode = YAML::Node(YAML::NodeType::Sequence);

		bool changed = false;

		ImGui::TextUnformatted("Collision Matrix");
		ImGui::Separator();

		ImGui::PushID("CollisionMatrix");
		const size_t layerCount = LayerManager::LayerCount();

		const float labelReservedWidth = 150.0f;

		static float lastMaxHeight = 150.0f;
		static float lastMatrixHeight = 150.0f;

		static int hoverXLastFrame = -1;
		static int hoverYLastFrame = -1;
		const int hoverX = hoverXLastFrame;
		const int hoverY = hoverYLastFrame;
		hoverXLastFrame = -1;
		hoverYLastFrame = -1;

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + labelReservedWidth);
		const ImVec2 startCursorPos = ImGui::GetCursorPos();

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + lastMaxHeight);

		const ImVec2 textSize = ImGui::CalcTextSize(" ");

		const ImVec2 cursorPos = ImGui::GetCursorPos();
		/* Render top layer labels first */
		for (size_t x = 0; x < layerCount; ++x)
		{
			const Layer* pXLayer = LayerManager::GetLayerAtIndex(x);
			const std::string& name = pXLayer->m_Name;

			const size_t textHeight = name.size() * textSize.y + 4.0f;

			if (textHeight > lastMaxHeight)
				lastMaxHeight = textHeight;

			const bool highlight = x == hoverX;

			ImGui::SetCursorPosX(cursorPos.x + x * 32.0f);
			ImGui::SetCursorPosY(startCursorPos.y);
			ImGui::Selectable("##back", highlight, ImGuiSelectableFlags_AllowItemOverlap, { 24.0f, lastMatrixHeight });
			ImGui::SetCursorPos(cursorPos);
			for (size_t i = name.size(); i > 0; --i)
			{
				const size_t index = name.size() - i;
				const char c = name[index];
				ImGui::SetCursorPosY(cursorPos.y - i * textSize.y);
				ImGui::SetCursorPosX(cursorPos.x + x * 32.0f);
				ImGui::TextColored({ highlight ? 0.0f : 1.0f, 1.0f, highlight ? 0.0f : 1.0f, 1.0f }, "%c", c);
			}
		}

		for (size_t y = 0; y < layerCount; ++y)
		{
			ImGui::PushID(y);

			const Layer* pYLayer = LayerManager::GetLayerAtIndex(y);
			for (size_t x = 0; x < layerCount; ++x)
			{
				ImGui::PushID(x);

				YAML::Node xNode = collisionMatrixNode[x];
				if (!xNode.IsSequence())
					xNode = YAML::Node(YAML::NodeType::Sequence);

				YAML::Node yNode = xNode[y];
				if (!yNode.IsDefined())
					yNode = true;

				const Layer* pXLayer = LayerManager::GetLayerAtIndex(x);
				bool value = yNode.as<bool>();

				if (!x)
				{
					const bool highlight = y == hoverY;

					ImGui::Selectable("##back", highlight, ImGuiSelectableFlags_AllowItemOverlap, { 0.0f, 24.0f });
					ImGui::SameLine();

					ImGui::TextColored({ highlight ? 0.0f : 1.0f, 1.0f, highlight ? 0.0f : 1.0f, 1.0f }, "%s", pYLayer->m_Name.c_str());
					const float maxWidth = ImGui::GetContentRegionAvail().x - labelReservedWidth;
					ImGui::SameLine();
					const float availableWidth = ImGui::GetContentRegionAvail().x;
					const float width = std::max(maxWidth, 100.0f);

					const ImVec2 cursorPos = ImGui::GetCursorPos();
					ImGui::SetCursorPos({ cursorPos.x + availableWidth - width, cursorPos.y });
				}
				else ImGui::SameLine();

				if (ImGui::Checkbox("##val", &value))
				{
					yNode = value;
					changed = true;
				}

				if (ImGui::IsItemHovered())
				{
					hoverXLastFrame = x;
					hoverYLastFrame = y;
				}

				ImGui::PopID();
			}
			ImGui::PopID();
		}
		ImGui::PopID();

		const float matrixHeight = ImGui::GetCursorPosY() - startCursorPos.y;
		if (matrixHeight > lastMatrixHeight) lastMatrixHeight = matrixHeight;

		ImGui::EndChild();

		return changed;
	}

	void PhysicsSettings::OnSettingsLoaded()
	{
	}

	void PhysicsSettings::OnStartPlay_Impl()
	{
		YAML::Node collisionMatrixNode = m_SettingsNode["CollisionMatrix"];
		if (!collisionMatrixNode.IsSequence())
			return;

		std::vector<std::vector<bool>> matrix;
		matrix.resize(LayerManager::LayerCount());

		for (size_t x = 0; x < collisionMatrixNode.size(); ++x)
		{
			matrix[x].resize(LayerManager::LayerCount());
			YAML::Node xNode = collisionMatrixNode[x];
			for (size_t y = 0; y < xNode.size(); ++y)
			{
				YAML::Node yNode = xNode[y];
				matrix[x][y] = yNode.as<bool>();
			}
		}

		Game::GetGame().GetEngine()->GetPhysicsModule()->SetCollisionMatrix(std::move(matrix));
	}
}

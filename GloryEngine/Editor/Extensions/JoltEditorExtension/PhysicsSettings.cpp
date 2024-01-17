#include "PhysicsSettings.h"

#include <EditorApplication.h>
#include <JoltPhysicsModule.h>
#include <Engine.h>
#include <YAML_GLM.h>
#include <LayerManager.h>
#include <imgui.h>
#include <EditorUI.h>

namespace Glory::Editor
{
	PhysicsSettings::PhysicsSettings() : ProjectSettings("Physics.yaml") {}

	bool PhysicsSettings::OnGui()
	{
		LayerManager& layers = EditorApplication::GetInstance()->GetEngine()->GetLayerManager();

		ImGui::BeginChild("Physics Settings");

		EditorUI::InputFloat3(m_YAMLFile, "Gravity");

		Utils::NodeValueRef collisionMatrixNode = m_YAMLFile["CollisionMatrix"];
		if (!collisionMatrixNode.IsSequence())
			collisionMatrixNode.Set(YAML::Node(YAML::NodeType::Sequence));

		bool changed = false;

		ImGui::TextUnformatted("Collision Matrix");
		ImGui::Separator();

		ImGui::PushID("CollisionMatrix");
		const size_t layerCount = layers.LayerCount();

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
			const Layer* pXLayer = layers.GetLayerAtIndex((int)x);
			const std::string& name = pXLayer->m_Name;

			const float textHeight = name.size() * textSize.y + 4.0f;

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
			ImGui::PushID((int)y);

			const Layer* pYLayer = layers.GetLayerAtIndex((int)y);
			for (size_t x = 0; x < layerCount; ++x)
			{
				ImGui::PushID((int)x);

				Utils::NodeValueRef xNode = collisionMatrixNode[x];
				if (!xNode.IsSequence())
					xNode.Set(YAML::Node(YAML::NodeType::Sequence));

				Utils::NodeValueRef yNode = xNode[y];
				if (!yNode.Exists())
					yNode.Set(true);

				const Layer* pXLayer = layers.GetLayerAtIndex((int)x);
				bool value = yNode.As<bool>();

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
					yNode.Set(value);
					changed = true;
				}

				if (ImGui::IsItemHovered())
				{
					hoverXLastFrame = (int)x;
					hoverYLastFrame = (int)y;
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
		LayerManager& layers = EditorApplication::GetInstance()->GetEngine()->GetLayerManager();

		Utils::NodeValueRef gravityNode = m_YAMLFile["Gravity"];
		const glm::vec3 gravity = gravityNode.As<glm::vec3>();

		Utils::NodeValueRef collisionMatrixNode = m_YAMLFile["CollisionMatrix"];
		if (!collisionMatrixNode.IsSequence())
			return;

		std::vector<std::vector<bool>> matrix;
		matrix.resize(layers.LayerCount());

		for (size_t x = 0; x < collisionMatrixNode.Size(); ++x)
		{
			matrix[x].resize(layers.LayerCount());
			Utils::NodeValueRef xNode = collisionMatrixNode[x];
			for (size_t y = 0; y < xNode.Size(); ++y)
			{
				Utils::NodeValueRef yNode = xNode[y];
				matrix[x][y] = yNode.As<bool>();
			}
		}

		JoltPhysicsModule* pPhysics = EditorApplication::GetInstance()->GetEngine()->GetOptionalModule<JoltPhysicsModule>();
		pPhysics->SetCollisionMatrix(std::move(matrix));
		pPhysics->SetGravity(gravity);
	}
}

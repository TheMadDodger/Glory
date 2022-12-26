#include "ListView.h"
#include <imgui.h>
#include "EditorUI.h"

#include "FontAwesome/IconsFontAwesome6.h"

namespace Glory::Editor
{
	ListView::ListView(const char* label) : m_Label(label)
	{
	}

	bool ListView::Draw(size_t elementCount)
	{
		bool change = false;

		ImGui::PushID(m_Label);

		const ImVec2 startPos = ImGui::GetCursorPos();
		const ImVec2 regionAvail = ImGui::GetContentRegionAvail();
		const bool open = ImGui::TreeNodeEx(EditorUI::MakeCleanName(m_Label).data(), ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed);
		const float buttonSize = 24.0f;
		if (OnResize != NULL)
		{
			ImGui::SetCursorPos({ startPos.x + regionAvail.x - buttonSize + 4.0f, startPos.y });
			int size = elementCount;
			ImGui::SetNextItemWidth(buttonSize);
			if (ImGui::InputInt("##size", &size, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				if (size <= 0) size = 0;
				elementCount = size;
				OnResize(elementCount);
				change = true;
			}
		}

		int toRemoveIndex = -1;
		if (open)
		{
			ImVec2 cursorPosBackup = ImGui::GetCursorPos();
			for (size_t i = 0; i < elementCount; i++)
			{
				ImGui::PushID(i);
				const ImVec2 regionAvailChild = ImGui::GetContentRegionAvail();
				const ImVec2 cursorPos = ImGui::GetCursorPos();
				//ImGui::BeginChild("##elementchild", { regionAvailChild.x - buttonSize - 2.0f, buttonSize }, false);
				OnDrawElement(i);
				//ImGui::EndChild();
				const ImVec2 cursorPosAfterElement = ImGui::GetCursorPos();
				cursorPosBackup = ImGui::GetCursorPos();
				ImGui::SetCursorPos({ cursorPos.x + regionAvailChild.x - buttonSize + 4.0f, cursorPos.y });
				if (ImGui::Button(ICON_FA_MINUS "##RemoveModule", { buttonSize, buttonSize }))
				{
					toRemoveIndex = i;
				}
				ImGui::PopID();
				ImGui::SetCursorPos(cursorPosAfterElement);
			}

			ImGui::TreePop();
			ImGui::SetCursorPos(cursorPosBackup);
			if (ImGui::Button("Add Element", { ImGui::GetContentRegionAvail().x, 0.0f }))
			{
				OnAdd();
				++elementCount;
				change = true;
			}
		}

		if (toRemoveIndex >= 0)
		{
			OnRemove(toRemoveIndex);
			change = true;
		}
		ImGui::PopID();
		return change;
	}
}

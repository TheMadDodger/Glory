#include "HistoryWindow.h"
#include "Undo.h"

namespace Glory::Editor
{
	HistoryWindow::HistoryWindow() : EditorWindowTemplate("History", 400.0f, 600.0f)
	{
		m_WindowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar;
	}

	HistoryWindow::~HistoryWindow()
	{
	}

	void HistoryWindow::OnGUI()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::MenuItem("Undo"))
				Undo::DoUndo();

			if (ImGui::MenuItem("Redo"))
				Undo::DoRedo();

			if (ImGui::MenuItem("Clear"))
				Undo::Clear();

			ImGui::EndMenuBar();
		}

		const ImVec2 maxRegion = ImGui::GetContentRegionAvail();
		const ImVec4 selectedColor = ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive);

		ImGui::BeginChild("##HistoryRegion", maxRegion, false);
		const ImVec2 historySize = ImGui::GetContentRegionAvail();
		ImGui::BeginListBox("##History", historySize);

		const size_t currentRewindIndex = Undo::CurrentRewindIndex();

		const size_t recordCount = Undo::GetHistorySize();
		const size_t selectedIndex = recordCount - currentRewindIndex;

		const float itemHeight = 24.0f;

		ImGuiListClipper clipper((int)recordCount, itemHeight);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });

		while (clipper.Step())
		{
			for (size_t i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
			{
				const size_t recordIndex = recordCount - i - 1;

				const ActionRecord* pRecord = Undo::RecordAt(recordIndex);
				const bool selected = recordIndex + 1 <= selectedIndex;
				ImGui::PushID((int)i);
				
				const float width = ImGui::GetContentRegionAvail().x;
				if (selected) ImGui::PushStyleColor(ImGuiCol_Button, selectedColor);
				ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.0f, 0.5f });
				if (ImGui::Button(pRecord->Name.c_str(), { width, 0.0f }))
				{
					Undo::JumpTo(i);
				}
				ImGui::PopStyleVar();
				if (selected) ImGui::PopStyleColor();
				
				ImGui::PopID();
			}
		}

		const float width = ImGui::GetContentRegionAvail().x;
		ImGui::PushStyleColor(ImGuiCol_Button, selectedColor);
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.0f, 0.5f });
		if (ImGui::Button("Start", {width, itemHeight }))
		{
			Undo::JumpTo(recordCount);
		}
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();

		ImGui::EndListBox();
		ImGui::EndChild();
	}
}

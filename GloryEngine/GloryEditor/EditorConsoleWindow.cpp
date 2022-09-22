#include "EditorConsoleWindow.h"
#include "Console.h"
#include "ImGuiHelpers.h"

namespace Glory::Editor
{
	EditorConsoleWindow::EditorConsoleWindow() : EditorWindowTemplate("Console", 1600.0f, 600.0f), m_InputBuffer(""), m_ScrollToBottom(false), m_NextColor(1.0f) {}

	EditorConsoleWindow::~EditorConsoleWindow()
	{
	}

	void EditorConsoleWindow::OnOpen()
	{
		Console::RegisterConsole(this);

        Debug::LogInfo("This is an info log!");
        Debug::LogNotice("This is a notice log!");
        Debug::LogWarning("This is a warning log!");
        Debug::LogError("This is an error log!");
	}

	void EditorConsoleWindow::OnClose()
	{
		Console::RemoveConsole(this);
	}

	void EditorConsoleWindow::OnGUI()
	{
		// Reserve enough left-over height for 1 separator + 1 input text
		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
		if (ImGui::BeginPopupContextWindow())
		{
			//if (ImGui::Selectable("Clear")) ClearLog();
			ImGui::EndPopup();
		}

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
        //if (copy_to_clipboard)
        //    ImGui::LogToClipboard();
        for (int i = 0; i < m_ConsoleLines.size(); i++)
        {
            const char* item = m_ConsoleLines[i].Message.data();
            if (!m_TextFilter.PassFilter(item))
                continue;

            ImVec4 color = glm::ToImVec4(m_ConsoleLines[i].Color);
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(item);
            ImGui::PopStyleColor();
        }
        //if (copy_to_clipboard)
        //    ImGui::LogFinish();

        if (m_ScrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        m_ScrollToBottom = false;

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::Separator();

        // Command-line
        bool reclaim_focus = false;
        ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue;// | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputText("##Input", m_InputBuffer, MAXINPUTLENGTH, input_text_flags))
        {
            std::string convertedInput = m_InputBuffer;
            Console::QueueCommand(convertedInput);
            strcpy(m_InputBuffer, "");
            reclaim_focus = true;
            m_ScrollToBottom = true;
        }
        ImGui::PopItemWidth();

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
	}

    //int EditorConsoleWindow::TextEditCallback(ImGuiInputTextCallbackData* data)
    //{
    //    //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
    //    switch (data->EventFlag)
    //    {
    //    case ImGuiInputTextFlags_CallbackCompletion:
    //    {
    //        // Example of TEXT COMPLETION
    //
    //        // Locate beginning of current word
    //        const char* word_end = data->Buf + data->CursorPos;
    //        const char* word_start = word_end;
    //        while (word_start > data->Buf)
    //        {
    //            const char c = word_start[-1];
    //            if (c == ' ' || c == '\t' || c == ',' || c == ';')
    //                break;
    //            word_start--;
    //        }
    //
    //        // Build a list of candidates
    //        ImVector<const char*> candidates;
    //        for (int i = 0; i < Commands.Size; i++)
    //            if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
    //                candidates.push_back(Commands[i]);
    //
    //        if (candidates.Size == 0)
    //        {
    //            // No match
    //            AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
    //        }
    //        else if (candidates.Size == 1)
    //        {
    //            // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
    //            data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
    //            data->InsertChars(data->CursorPos, candidates[0]);
    //            data->InsertChars(data->CursorPos, " ");
    //        }
    //        else
    //        {
    //            // Multiple matches. Complete as much as we can..
    //            // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
    //            int match_len = (int)(word_end - word_start);
    //            for (;;)
    //            {
    //                int c = 0;
    //                bool all_candidates_matches = true;
    //                for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
    //                    if (i == 0)
    //                        c = toupper(candidates[i][match_len]);
    //                    else if (c == 0 || c != toupper(candidates[i][match_len]))
    //                        all_candidates_matches = false;
    //                if (!all_candidates_matches)
    //                    break;
    //                match_len++;
    //            }
    //
    //            if (match_len > 0)
    //            {
    //                data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
    //                data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
    //            }
    //
    //            // List matches
    //            AddLog("Possible matches:\n");
    //            for (int i = 0; i < candidates.Size; i++)
    //                AddLog("- %s\n", candidates[i]);
    //        }
    //
    //        break;
    //    }
    //    case ImGuiInputTextFlags_CallbackHistory:
    //    {
    //        // Example of HISTORY
    //        const int prev_history_pos = HistoryPos;
    //        if (data->EventKey == ImGuiKey_UpArrow)
    //        {
    //            if (HistoryPos == -1)
    //                HistoryPos = History.Size - 1;
    //            else if (HistoryPos > 0)
    //                HistoryPos--;
    //        }
    //        else if (data->EventKey == ImGuiKey_DownArrow)
    //        {
    //            if (HistoryPos != -1)
    //                if (++HistoryPos >= History.Size)
    //                    HistoryPos = -1;
    //        }
    //
    //        // A better implementation would preserve the data on the current input line along with cursor position.
    //        if (prev_history_pos != HistoryPos)
    //        {
    //            const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
    //            data->DeleteChars(0, data->BufTextLen);
    //            data->InsertChars(0, history_str);
    //        }
    //    }
    //    }
    //    return 0;
    //}

	void EditorConsoleWindow::Initialize()
	{
	}

	void EditorConsoleWindow::OnConsoleClose()
	{
        m_ConsoleLines.clear();
	}

	void EditorConsoleWindow::SetNextColor(const glm::vec4& color)
	{
        m_NextColor = color;
	}

	void EditorConsoleWindow::ResetNextColor()
	{
        m_NextColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	}

	void EditorConsoleWindow::Write(const std::string& line)
	{
        m_ConsoleLines.push_back({ line, m_NextColor });
        ResetNextColor();
	}

    EditorConsoleWindow::ConsoleLine::ConsoleLine(const std::string& message, const glm::vec4& color) : Message(message), Color(color) {}
}

#pragma once
#include "EditorWindow.h"
#include "IConsole.h"
#include <imgui.h>

namespace Glory::Editor
{
    class EditorConsoleWindow : public EditorWindowTemplate<EditorConsoleWindow>, IConsole
    {
    public:
        EditorConsoleWindow();
        virtual ~EditorConsoleWindow();

        virtual void OnOpen() override;
        virtual void OnClose() override;

    private:
        virtual void OnGUI() override;

        void Initialize() override;
        void OnConsoleClose() override;
        void SetNextColor(const glm::vec4& color) override;
        void ResetNextColor() override;
        void Write(const std::string& line) override;

        // Callback for when we add history and autocomplete to the console system
        //int TextEditCallback(ImGuiInputTextCallbackData* data);

        struct ConsoleLine
        {
            ConsoleLine(const std::string& message, const glm::vec4& color);
            const std::string Message;
            const glm::vec4 Color;
        };

        std::vector<ConsoleLine> m_ConsoleLines;
        static const int MAXINPUTLENGTH = 200;
        char m_InputBuffer[MAXINPUTLENGTH];
        ImGuiTextFilter m_TextFilter;
        bool m_ScrollToBottom;
        glm::vec4 m_NextColor;
    };
}

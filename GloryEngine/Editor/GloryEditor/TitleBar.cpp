#include "TitleBar.h"
#include <Engine.h>
#include <sstream>

namespace Glory::Editor
{
    std::vector<TitleBarSection> TitleBar::m_Sections;
    std::map<std::string_view, size_t> TitleBar::m_SectionNameToIndex;

    TitleBarSection::TitleBarSection(const char* name, const char* defaultValue)
        : m_Name(name), m_Text(defaultValue)
    {
    }

    void TitleBar::AddSection(const char* name, const char* defaultValue)
    {
        const size_t index = m_Sections.size();
        m_Sections.emplace_back(TitleBarSection{ name, defaultValue });
        m_SectionNameToIndex.emplace(m_Sections[index].m_Name, index);
    }

    void TitleBar::SetText(std::string_view name, const char* text)
    {
        if (m_SectionNameToIndex.find(name) == m_SectionNameToIndex.end()) return;
        size_t index = m_SectionNameToIndex.at(name);
        m_Sections[index].m_Text = text;
        UpdateTitlebarText();
    }

    void TitleBar::UpdateTitlebarText()
    {
        Window* pMainWindow = Game::GetGame().GetEngine()->GetWindowModule()->GetMainWindow();

        std::stringstream stream;
        for (size_t i = 0; i < m_Sections.size(); i++)
        {
            stream << m_Sections[i].m_Text;
        }

        pMainWindow->SetWindowTitle(stream.str().c_str());
    }
}

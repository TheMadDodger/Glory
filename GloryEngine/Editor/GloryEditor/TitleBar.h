#pragma once
#include <vector>
#include <map>
#include <string>
#include <string_view>
#include "GloryEditor.h"

namespace Glory::Editor
{
	struct TitleBarSection
	{
		TitleBarSection(const char* name, const char* defaultValue);

		const char* m_Name;
		std::string m_Text;
	};

	class TitleBar
	{
	public:
		GLORY_EDITOR_API static void AddSection(const char* name, const char* defaultValue);
		GLORY_EDITOR_API static void SetText(std::string_view name, const char* text);
		GLORY_EDITOR_API static void UpdateTitlebarText();

	private:
		static std::vector<TitleBarSection> m_Sections;
		static std::map<std::string_view, size_t> m_SectionNameToIndex;

	private:
		TitleBar() = delete;
	};
}

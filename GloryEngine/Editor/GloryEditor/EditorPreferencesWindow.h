#pragma once
#include "EditorWindow.h"
#include <functional>
#include <string>

namespace Glory::Editor
{
	struct PreferencesTab
	{
		const std::string m_Name;
		const std::function<void()> m_DrawCallback;
	};

	class EditorPreferencesWindow : public EditorWindowTemplate<EditorPreferencesWindow>
	{
	public:
		EditorPreferencesWindow();
		virtual ~EditorPreferencesWindow();

		GLORY_EDITOR_API static void AddPreferencesTab(const PreferencesTab&& tab);

	private:
		virtual void OnGUI() override;
		void OnClose() override;
		void UpdateTheme();
		void AppearanceTab();

		void Shortcuts();

	private:
		static int m_ThemeIndex;
		std::string_view m_RebindingShortcut;
		static std::vector<PreferencesTab> m_Tabs;
	};
}
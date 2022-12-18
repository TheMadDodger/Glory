#pragma once
#include "EditorWindow.h"

namespace Glory::Editor
{
	class EditorPreferencesWindow : public EditorWindowTemplate<EditorPreferencesWindow>
	{
	public:
		EditorPreferencesWindow();
		virtual ~EditorPreferencesWindow();

	private:
		virtual void OnGUI() override;
		void OnClose() override;
		void UpdateTheme();
		void AppearanceTab();

		void Shortcuts();

	private:
		static int m_ThemeIndex;
		std::string_view m_RebindingShortcut;
	};
}
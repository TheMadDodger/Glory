#pragma once
#include "GloryEditor.h"

namespace Glory::Editor
{
	class PopupManager
	{
	private:
		struct Popup
		{
			Popup(const std::string& name, const std::string& description, const std::vector<std::string>& buttons, const std::vector<std::function<void()>>& buttonFuncs, bool modal = false);

			std::string m_PopupName;
			std::string m_PopupDescription;
			std::vector<std::string> m_Buttons;
			std::vector<std::function<void()>> m_ButtonFuncs;
			bool m_Modal;
		};

	public:
		static GLORY_EDITOR_API void OpenPopup(const std::string& name, const std::string& description, const std::vector<std::string>& buttons, const std::vector<std::function<void()>>& buttonFuncs);
		static GLORY_EDITOR_API void OpenModal(const std::string& name, const std::string& description, const std::vector<std::string>& buttons, const std::vector<std::function<void()>>& buttonFuncs);
		static GLORY_EDITOR_API void CloseCurrentPopup();

	private:
		friend class MainEditor;
		static void OnGUI();

		static void PopupGUI(const Popup& popup);
		static void ModalGUI(const Popup& popup);
		static void PopupBody(const Popup& popup);

		static std::vector<Popup> m_Popups;
		static std::vector<std::string> m_OpeningPopupNames;
		static std::vector<std::string> m_OpeningModalNames;

	private:
		PopupManager();
		virtual ~PopupManager();
	};
}
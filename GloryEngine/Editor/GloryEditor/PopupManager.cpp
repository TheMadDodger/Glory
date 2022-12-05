#include <imgui.h>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include "PopupManager.h"

namespace Glory::Editor
{
	std::vector<PopupManager::Popup> PopupManager::m_Popups = std::vector<Popup>();
	std::vector<std::string> PopupManager::m_OpeningPopupNames = std::vector<std::string>();

	GLORY_EDITOR_API void PopupManager::OpenPopup(const std::string& name, const std::string& description, const std::vector<std::string>& buttons, const std::vector<std::function<void()>>& buttonFuncs)
	{
		m_OpeningPopupNames.push_back(name);

		auto it = std::find_if(m_Popups.begin(), m_Popups.end(), [&](const Popup& popup)
			{
				return popup.m_PopupName == name;
			});

		if (it != m_Popups.end()) return;
		m_Popups.push_back(Popup(name, description, buttons, buttonFuncs));
	}

	GLORY_EDITOR_API void PopupManager::OpenModal(const std::string& name, const std::string& description, const std::vector<std::string>& buttons, const std::vector<std::function<void()>>& buttonFuncs)
	{
		m_OpeningPopupNames.push_back(name);

		auto it = std::find_if(m_Popups.begin(), m_Popups.end(), [&](const Popup& popup)
			{
				return popup.m_PopupName == name;
			});

		if (it != m_Popups.end()) return;
		m_Popups.push_back(Popup(name, description, buttons, buttonFuncs, true));
	}

	GLORY_EDITOR_API void PopupManager::CloseCurrentPopup()
	{
		ImGui::CloseCurrentPopup();
	}

	void PopupManager::OnGUI()
	{
		std::for_each(m_OpeningPopupNames.begin(), m_OpeningPopupNames.end(), [](const std::string& name)
			{
				ImGui::OpenPopup(name.c_str());
			});
		m_OpeningPopupNames.clear();

		std::for_each(m_Popups.begin(), m_Popups.end(), [&](const Popup& popup)
		{
			if (popup.m_Modal) ModalGUI(popup);
			else PopupGUI(popup);
		});
	}

	void PopupManager::PopupGUI(const Popup& popup)
	{
		if (ImGui::BeginPopup(popup.m_PopupName.c_str()))
		{
			PopupBody(popup);
			ImGui::EndPopup();
		}
	}

	void PopupManager::ModalGUI(const Popup& popup)
	{
		if (ImGui::BeginPopupModal(popup.m_PopupName.c_str()))
		{
			PopupBody(popup);
			ImGui::EndPopup();
		}
	}

	void PopupManager::PopupBody(const Popup& popup)
	{
		ImGui::Text(popup.m_PopupName.c_str());
		ImGui::Text(popup.m_PopupDescription.c_str());

		for (size_t i = 0; i < popup.m_Buttons.size(); i++)
		{
			if (ImGui::Button(popup.m_Buttons[i].c_str()))
			{
				if (i < popup.m_ButtonFuncs.size())
				{
					popup.m_ButtonFuncs[i]();
				}
			}
			ImGui::SameLine();
		}
	}

	PopupManager::PopupManager()
	{
	}

	PopupManager::~PopupManager()
	{
	}

	PopupManager::Popup::Popup(const std::string& name, const std::string& description, const std::vector<std::string>& buttons, const std::vector<std::function<void()>>& buttonFuncs, bool modal)
		: m_PopupName(name), m_PopupDescription(description), m_Buttons(buttons), m_ButtonFuncs(buttonFuncs), m_Modal(modal)
	{
	}
}
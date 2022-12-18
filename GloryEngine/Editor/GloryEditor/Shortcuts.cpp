#include "Shortcuts.h"
#include <Debug.h>

namespace Glory::Editor
{
	std::map<std::string_view, Shortcut> Shortcuts::m_Shortcuts;

	Shortcut::Shortcut() :
		m_Name("INVALID"), m_Action(NULL), m_Key(ImGuiKey_None), m_Mods(ImGuiModFlags_None)
	{
	}

	Shortcut::Shortcut(const char* action, std::function<void()> callback)
		: m_Name(action), m_Action(callback), m_Key(ImGuiKey_None), m_Mods(ImGuiModFlags_None)
	{}

	const Shortcut* Shortcuts::AddAction(const char* action, std::function<void()> callback)
	{
		if (m_Shortcuts.find(action) != m_Shortcuts.end())
		{
			Debug::LogWarning(std::string("Shortcut action ") + action + " already exists.");
			return nullptr;
		}

		m_Shortcuts.emplace(action, Shortcut{ action, callback });
		return &m_Shortcuts.at(action);
	}

	const Shortcut* Shortcuts::GetShortcut(std::string_view action)
	{
		if (m_Shortcuts.find(action) == m_Shortcuts.end()) return nullptr;
		return &m_Shortcuts.at(action);
	}

	void Shortcuts::SetShortcut(const char* action, ImGuiKey key, ImGuiModFlags mods)
	{
		if (m_Shortcuts.find(action) == m_Shortcuts.end()) return;
		m_Shortcuts[action].m_Key = key;
		m_Shortcuts[action].m_Mods = mods;
	}

	std::string Shortcuts::GetShortcutString(std::string_view action)
	{
		const Shortcut* shortcut = GetShortcut(action);
		if (!shortcut) return "";

		std::string shortcutString;
		/* Check all mods */
		if (shortcut->m_Mods & ImGuiModFlags_Ctrl)
			shortcutString += "Ctrl + ";
		if (shortcut->m_Mods & ImGuiModFlags_Shift)
			shortcutString += "Shift + ";
		if (shortcut->m_Mods & ImGuiModFlags_Alt)
			shortcutString += "Alt + ";
		if (shortcut->m_Mods & ImGuiModFlags_Super)
			shortcutString += "Super + ";

		/* Add key */
		shortcutString += ImGui::GetKeyName(shortcut->m_Key);
		return shortcutString;
	}

	void Shortcuts::Clear()
	{
		m_Shortcuts.clear();
	}

	void Shortcuts::SaveShortcuts()
	{
	}

	void Shortcuts::LoadShortcuts()
	{
	}

	void Shortcuts::Update()
	{
		ImGuiIO& io = ImGui::GetIO();

		for (auto itor = m_Shortcuts.begin(); itor != m_Shortcuts.end(); ++itor)
		{
			if (!ImGui::IsKeyPressed(itor->second.m_Key)) continue;
			/* Mods need to be the exact same to prevent triggering multiple shortcuts. */
			if (io.KeyMods != itor->second.m_Mods) continue;
			itor->second.m_Action();
			/* Can only trigger 1 action per frame! */
			return;
		}
	}
}
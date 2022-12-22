#include "Shortcuts.h"
#include <Debug.h>

namespace Glory::Editor
{
	std::map<std::string_view, Shortcut> Shortcuts::m_Shortcuts;
	std::vector<ImGuiKey> Shortcuts::m_CurrentBlockedKeys;

	Shortcut::Shortcut() :
		m_Name("INVALID"), m_Action(NULL), m_Key(ImGuiKey_None), m_Mods(ImGuiModFlags_None), m_Blocked(false)
	{}

	Shortcut::Shortcut(const char* action, std::function<void()> callback)
		: m_Name(action), m_Action(callback), m_Key(ImGuiKey_None), m_Mods(ImGuiModFlags_None), m_Blocked(false)
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

	void Shortcuts::SetShortcut(std::string_view action, ImGuiKey key, ImGuiModFlags mods)
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

	const std::map<std::string_view, Shortcut>::iterator Shortcuts::Begin()
	{
		return m_Shortcuts.begin();
	}

	const std::map<std::string_view, Shortcut>::iterator Shortcuts::End()
	{
		return m_Shortcuts.end();
	}

	void Shortcuts::SaveShortcuts(YAML::Emitter& out)
	{
		out << YAML::Key << "Shortcuts";
		out << YAML::BeginSeq;
		for (auto itor = m_Shortcuts.begin(); itor != m_Shortcuts.end(); ++itor)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Name";
			out << YAML::Value << itor->first.data();
			out << YAML::Key << "Key";
			out << YAML::Value << int(itor->second.m_Key);
			out << YAML::Key << "Mods";
			out << YAML::Value << int(itor->second.m_Mods);
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
	}

	void Shortcuts::LoadShortcuts(YAML::Node& node)
	{
		YAML::Node shortcutsNode = node["Shortcuts"];
		if (!shortcutsNode.IsDefined() || !shortcutsNode.IsSequence()) return;
		for (size_t i = 0; i < shortcutsNode.size(); i++)
		{
			YAML::Node shortcutNode = shortcutsNode[i];
			std::string name = shortcutNode["Name"].as<std::string>();
			ImGuiKey key = ImGuiKey(shortcutNode["Key"].as<int>());
			ImGuiModFlags mods = shortcutNode["Mods"].as<int>();
			Shortcuts::SetShortcut(name, key, mods);
		}
	}

	void Shortcuts::BlockActionForOneFrame(std::string_view action)
	{
		if (m_Shortcuts.find(action) == m_Shortcuts.end()) return;
		m_Shortcuts.at(action).m_Blocked = true;
	}

	void Shortcuts::AddBlockKeyForOneFrame(ImGuiKey key)
	{
		m_CurrentBlockedKeys.push_back(key);
	}

	void Shortcuts::Clear()
	{
		m_Shortcuts.clear();
		m_CurrentBlockedKeys.clear();
	}

	void Shortcuts::Update()
	{
		ImGuiIO& io = ImGui::GetIO();

		for (auto itor = m_Shortcuts.begin(); itor != m_Shortcuts.end(); ++itor)
		{
			const bool keyBlocked = std::find(m_CurrentBlockedKeys.begin(), m_CurrentBlockedKeys.end(), itor->second.m_Key) != m_CurrentBlockedKeys.end();

			if (itor->second.m_Blocked || keyBlocked)
			{
				/* Reset blocked state */
				itor->second.m_Blocked = false;
				continue;
			}

			if (!ImGui::IsKeyPressed(itor->second.m_Key)) continue;
			/* Mods need to be the exact same to prevent triggering multiple shortcuts. */
			if (io.KeyMods != itor->second.m_Mods) continue;
			itor->second.m_Action();
		}

		m_CurrentBlockedKeys.clear();
	}
}
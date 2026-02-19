#include "Shortcuts.h"
#include "EditorApplication.h"

#include <Debug.h>

namespace Glory::Editor
{
	std::map<std::string_view, Shortcut> Shortcuts::m_Shortcuts;
	std::vector<ImGuiKey> Shortcuts::m_CurrentBlockedKeys;
	std::vector<std::string_view> Shortcuts::m_TriggeredThisFame;

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
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogWarning(std::string("Shortcut action ") + action + " already exists.");
			return nullptr;
		}

		m_Shortcuts.emplace(action, Shortcut{ action, callback });
		return &m_Shortcuts.at(action);
	}

	const Shortcut* Shortcuts::AddMainWindowAction(const char* action, size_t index, std::function<void()> callback)
	{
		auto& iter = m_Shortcuts.find(action);

		if (iter == m_Shortcuts.end())
		{
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogWarning(std::string("Shortcut action ") + action + " does not exist.");
			return nullptr;
		}

		if (iter->second.m_MainWindowActions.size() <= index)
			iter->second.m_MainWindowActions.resize(index + 1, NULL);

		iter->second.m_MainWindowActions[index] = callback;
		return &iter->second;
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

	void Shortcuts::SaveShortcuts(Utils::YAMLFileRef& yamlFile)
	{
		Utils::NodeValueRef shortcuts = yamlFile["Shortcuts"];
		if (!shortcuts.IsSequence())
		{
			shortcuts.Set(YAML::Node(YAML::NodeType::Sequence));
		}

		size_t index = 0;
		for (auto itor = m_Shortcuts.begin(); itor != m_Shortcuts.end(); ++itor)
		{
			Utils::NodeValueRef shortcut = shortcuts[index];
			if (!shortcut.IsMap())
			{
				shortcut.Set(YAML::Node(YAML::NodeType::Map));
			}

			shortcut["Name"].Set(itor->first.data());
			shortcut["Key"].Set((int)itor->second.m_Key);
			shortcut["Mods"].Set((int)itor->second.m_Mods);

			++index;
		}
	}

	void Shortcuts::LoadShortcuts(Utils::YAMLFileRef& yamlFile)
	{
		Utils::NodeValueRef shortcuts = yamlFile["Shortcuts"];
		if (!shortcuts.IsSequence()) return;
		for (size_t i = 0; i < shortcuts.Size(); i++)
		{
			Utils::NodeValueRef shortcut = shortcuts[i];
			std::string name = shortcut["Name"].As<std::string>("");
			ImGuiKey key = ImGuiKey(shortcut["Key"].As<int>(0));
			ImGuiModFlags mods = shortcut["Mods"].As<int>(0);
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

	bool Shortcuts::IsActionTriggered(std::string_view action)
	{
		return std::find(m_TriggeredThisFame.begin(), m_TriggeredThisFame.end(), action) != m_TriggeredThisFame.end();
	}

	void Shortcuts::Clear()
	{
		m_Shortcuts.clear();
		m_CurrentBlockedKeys.clear();
	}

	void Shortcuts::Update(size_t currentMainWindowIndex)
	{
		m_TriggeredThisFame.clear();

		/* If any ImGui item is active we want to ignore all shortcuts */
		if (ImGui::IsAnyItemActive())
			return;

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

			if (!ImGui::IsKeyPressed(itor->second.m_Key, false))
			{
				continue;
			}
			/* Mods need to be the exact same to prevent triggering multiple shortcuts. */
			if (io.KeyMods != itor->second.m_Mods)
			{
				continue;
			}

			if (itor->second.m_MainWindowActions.size() > currentMainWindowIndex)
			{
				auto& mainWindowCallback = itor->second.m_MainWindowActions[currentMainWindowIndex];
				if (mainWindowCallback)
				{
					mainWindowCallback();
					return;
				}
			}
			if (itor->second.m_Action) itor->second.m_Action();
			m_TriggeredThisFame.emplace_back(itor->second.m_Name);
		}

		m_CurrentBlockedKeys.clear();
	}
}
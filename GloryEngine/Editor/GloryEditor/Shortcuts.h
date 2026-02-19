#pragma once
#include "GloryEditor.h"

#include <imgui.h>
#include <map>
#include <vector>
#include <string_view>
#include <functional>
#include <yaml-cpp/yaml.h>
#include <NodeRef.h>

namespace Glory::Editor
{
	/* @brief Defines a shortcut that will trigger an action */
	struct Shortcut
	{
		Shortcut();
		Shortcut(const char* action, std::function<void()> callback);

		const char* m_Name;
		const std::function<void()> m_Action;
		std::vector<std::function<void()>> m_MainWindowActions;
		ImGuiKey m_Key;
		ImGuiModFlags m_Mods;
		bool m_Blocked;
	};

	/* @brief Manager to handle all shortcuts and their set keys */
	class Shortcuts
	{
	public:
		/*
		 * @brief Adds an action and binds it to the provided function.
		 * If the action already exists then an error will be thrown.
		 * @param name The name of the action
		 * @param callback The function to bind to the action
		 */
		static GLORY_EDITOR_API const Shortcut* AddAction(const char* action, std::function<void()> callback);

		/*
		 * @brief Adds a window specific action to an existing shortcut.
		 * If the action does not exist then an error will be thrown.
		 * @param name The name of the action
		 * @param index Index of the main window
		 * @param callback The function to bind to the action
		 */
		static GLORY_EDITOR_API const Shortcut* AddMainWindowAction(const char* action, size_t index, std::function<void()> callback);

		/*
		 * @brief Gets the Shortcut belonging to the action.
		 * @param action The name of the action to get the shortcut from
		 * @returns A pointer to the Shortcut or nullptr if it does not exist.
		 */
		static GLORY_EDITOR_API const Shortcut* GetShortcut(std::string_view action);

		/*
		 * @brief Sets the shortcut for the chosen action.
		 * @param action The name of the action the shortcut should trigger.
		 * @param key The main key that triggers the action.
		 * @param mods The modifiers that need to be active on the key to trigger the action.
		 */
		static GLORY_EDITOR_API void SetShortcut(std::string_view action, ImGuiKey key, ImGuiModFlags mods);

		/*
		 * @brief Gets a formatted string of the shortcut (Mod1 + ModN + Key).
		 * @param action The name of the action to get the shortcut string from.
		 */
		static GLORY_EDITOR_API std::string GetShortcutString(std::string_view action);

		/* @brief Begin iterator of Shortcut map */
		static GLORY_EDITOR_API const std::map<std::string_view, Shortcut>::iterator Begin();
		/* @brief End iterator of Shortcut map */
		static GLORY_EDITOR_API const std::map<std::string_view, Shortcut>::iterator End();

		/*
		 * @brief Write user set shortcut keys to a yaml emitter.
		 * @param yamlFile The yaml file to write the keys to.
		 */
		static GLORY_EDITOR_API void SaveShortcuts(Utils::YAMLFileRef& yamlFile);
		/*
		 * @brief Read user set shortcut keys from a yaml node.
		 * @param yamlFile The yaml file to read the keys from.
		 */
		static GLORY_EDITOR_API void LoadShortcuts(Utils::YAMLFileRef& yamlFile);

		/*
		 * @brief Block an action from being triggered for one frame.
		 * @param action The action to block.
		 */
		static GLORY_EDITOR_API void BlockActionForOneFrame(std::string_view action);
		/*
		 * @brief Block a key from activating any actions for one frame.
		 * @param action The key to block.
		 */
		static GLORY_EDITOR_API void AddBlockKeyForOneFrame(ImGuiKey key);
		/*
		 * @brief Check whether an action was triggered this frame.
		 * @param action The action to check for.
		 */
		static GLORY_EDITOR_API bool IsActionTriggered(std::string_view action);


	private:
		/* @brief Clears all shortcuts and actions, used for cleanup. */
		static void Clear();

		/* @brief Check for triggered shortcuts this frame. */
		static void Update(size_t currentMainWindowIndex);

	private:
		Shortcuts() = delete;

	private:
		friend class MainEditor;
		static std::map<std::string_view, Shortcut> m_Shortcuts;
		static std::vector<ImGuiKey> m_CurrentBlockedKeys;
		static std::vector<std::string_view> m_TriggeredThisFame;
	};
}

#pragma once
#include <imgui.h>
#include <map>
#include <vector>
#include <string_view>
#include <functional>
#include "GloryEditor.h"

namespace Glory::Editor
{
	/* Defines a shortcut that will trigger an action */
	struct Shortcut
	{
		Shortcut();
		Shortcut(const char* action, std::function<void()> callback);

		const char* m_Name;
		const std::function<void()> m_Action;
		ImGuiKey m_Key;
		ImGuiModFlags m_Mods;
	};

	class Shortcuts
	{
	public:
		/* @brief Adds an action and binds it to the provided function.
		 * If the action already exists then an error will be thrown.
		 * @param name The name of the action
		 * @param callback The function to bind to the action */
		static GLORY_EDITOR_API const Shortcut* AddAction(const char* action, std::function<void()> callback);

		/* @brief Gets the Shortcut belonging to the action.
		 * @param action The name of the action to get the shortcut from
		 * @returns A pointer to the Shortcut or nullptr if it does not exist. */
		static GLORY_EDITOR_API const Shortcut* GetShortcut(std::string_view action);

		/* @brief Sets the shortcut for the chosen action.
		 * @param action The name of the action the shortcut should trigger.
		 * @param key The main key that triggers the action.
		 * @param mods The modifiers that need to be active on the key to trigger the action. */
		static GLORY_EDITOR_API void SetShortcut(const char* action, ImGuiKey key, ImGuiModFlags mods);

		static GLORY_EDITOR_API std::string GetShortcutString(std::string_view action);

	private:
		/* @brief Clears all shortcuts and actions, used for cleanup. */
		static void Clear();

		static void SaveShortcuts();
		/* @brief Loads the user defined shortcuts. */
		static void LoadShortcuts();

		/* @brief Check for triggered shortcuts this frame. */
		static void Update();

	private:
		Shortcuts() = delete;

	private:
		friend class MainEditor;
		static std::map<std::string_view, Shortcut> m_Shortcuts;
	};
}

#pragma once
#include <string>
#include <vector>
#include <functional>
#include "EditorWindow.h"

namespace Glory::Editor
{
	class MenuBar
	{
	public:
		static void AddMenuItem(std::string path, std::function<void()> func, std::function<bool()> selectedFunc = NULL);

	private:
		static void OnGUI();

		static std::vector<std::string> DisectPath(const std::string& path);

		struct MenuItem
		{
		public:
			MenuItem(const std::string& name);
			MenuItem(const std::string& name, std::function<void()> func, std::function<bool()> selectedFunc = NULL);

			const std::string m_Name;
			std::vector<MenuItem> m_Children;
			bool m_HasFunc;
			std::function<void()> m_Func;
			std::function<bool()> m_SelectedFunc;
		};

		static MenuItem* GetMenuItem(std::vector<MenuItem>& menuItems, const std::string& name);
		static void MenusRecursive(const MenuItem& menuItem);

	private:
		friend class MainEditor;
		static std::vector<MenuItem> m_MenuItems;
		static float m_MenuBarHeight;

	private:
		MenuBar();
		virtual ~MenuBar();
	};
}
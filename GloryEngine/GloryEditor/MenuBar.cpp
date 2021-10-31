#include "MenuBar.h"
#include <algorithm>
#include <imgui.h>

namespace Glory::Editor
{
	std::vector<MenuBar::MenuItem> MenuBar::m_MenuItems = std::vector<MenuItem>();
	float MenuBar::m_MenuBarHeight = 0.0f;

	void MenuBar::AddMenuItem(std::string path, std::function<void()> func)
	{
		std::vector<std::string> items = DisectPath(path);

		MenuItem* currentItem = GetMenuItem(m_MenuItems, items[0]);
		std::for_each(items.begin() + 1, items.end(), [&](const std::string& item)
			{
				currentItem = GetMenuItem(currentItem->m_Children, item);
			});

		currentItem->m_Func = func;
		currentItem->m_HasFunc = true;
	}

	void MenuBar::OnGUI()
	{
		ImGui::BeginMainMenuBar();
		ImVec2 size = ImGui::GetWindowSize();
		m_MenuBarHeight = size.y;

		std::for_each(m_MenuItems.begin(), m_MenuItems.end(), [](const MenuBar::MenuItem& childItem)
		{
			if (childItem.m_HasFunc)
			{
				if (ImGui::MenuItem(childItem.m_Name.c_str()))
					childItem.m_Func();
			}
			else if (ImGui::BeginMenu(childItem.m_Name.c_str()))
			{
				MenusRecursive(childItem);
				ImGui::EndMenu();
			}
		});
		ImGui::EndMainMenuBar();
	}

	std::vector<std::string> MenuBar::DisectPath(const std::string& path)
	{
		std::vector<std::string> result;
		std::string currentPath = path;

		int forwardSlashIndex = currentPath.find("/");
		while (forwardSlashIndex != std::string::npos)
		{
			std::string item = currentPath.substr(0, forwardSlashIndex);
			result.push_back(item);
			currentPath = currentPath.substr(forwardSlashIndex + 1);
			forwardSlashIndex = currentPath.find("/");
		}
		result.push_back(currentPath);
		return result;
	}

	MenuBar::MenuItem* MenuBar::GetMenuItem(std::vector<MenuItem>& menuItems, const std::string& name)
	{
		auto it = std::find_if(menuItems.begin(), menuItems.end(), [name](MenuItem& menuItem)
			{
				return menuItem.m_Name == name;
			});

		if (it != menuItems.end())
		{
			MenuItem* p = &*it;
			return p;
		}

		menuItems.push_back(MenuItem(name));
		MenuItem* newItem = &menuItems[menuItems.size() - 1];
		return newItem;
	}

	void MenuBar::MenusRecursive(const MenuItem& menuItem)
	{
		std::for_each(menuItem.m_Children.begin(), menuItem.m_Children.end(), [](const MenuBar::MenuItem& childItem)
			{
				if (childItem.m_HasFunc)
				{
					if (ImGui::MenuItem(childItem.m_Name.c_str()))
						childItem.m_Func();
				}
				else if (ImGui::BeginMenu(childItem.m_Name.c_str()))
				{
					MenusRecursive(childItem);
					ImGui::EndMenu();
				}
			});
	}

	MenuBar::MenuBar()
	{
	}

	MenuBar::~MenuBar()
	{
	}

	MenuBar::MenuItem::MenuItem(const std::string& name) : m_Name(name), m_HasFunc(false), m_Func()
	{
	}

	MenuBar::MenuItem::MenuItem(const std::string& name, std::function<void()> func) : m_Name(name), m_HasFunc(true), m_Func(func)
	{
	}
}
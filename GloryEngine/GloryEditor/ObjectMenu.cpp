#include "ObjectMenu.h"
#include <ResourceType.h>
#include <GScene.h>
#include <algorithm>
#include <imgui.h>

namespace Glory::Editor
{
	Object* ObjectMenu::m_pObject = nullptr;
	std::vector<ObjectMenuItem> ObjectMenu::m_MenuItems = std::vector<ObjectMenuItem>();
	ObjectMenuType ObjectMenu::m_CurrentMenuType = T_Undefined;
	bool ObjectMenu::m_Open = false;

	void ObjectMenu::Open(Object* pObject, ObjectMenuType forceMenuType)
	{
		m_pObject = pObject;
		m_CurrentMenuType = forceMenuType;
		m_Open = true;
	}

	void ObjectMenu::AddMenuItem(std::string path, std::function<void(Object*, const ObjectMenuType&)> func, const ObjectMenuTypeFlags& relevantMenus)
	{
		std::vector<std::string> items = DisectPath(path);

		ObjectMenuItem* currentItem = GetMenuItem(m_MenuItems, items[0], (ObjectMenuType)relevantMenus);
		std::for_each(items.begin() + 1, items.end(), [&](const std::string& item)
		{
			currentItem = GetMenuItem(currentItem->m_Children, item, (ObjectMenuType)relevantMenus);
		});

		currentItem->m_Func = func;
	}

	void ObjectMenu::OnGUI()
	{
		if (m_Open)
		{
			ImGui::OpenPopup("object_menu_popup");
			m_Open = false;
		}

		if (ImGui::BeginPopup("object_menu_popup"))
		{
			ImGui::MenuItem("Object Menu", NULL, false, false);

			if (m_CurrentMenuType == T_Undefined)
			{
				ImGui::EndPopup();
				return;
			}

			std::for_each(m_MenuItems.begin(), m_MenuItems.end(), [&](const ObjectMenuItem& childItem)
			{
				if (childItem.m_RelevantMenus != T_Undefined && (childItem.m_RelevantMenus & m_CurrentMenuType) == 0) return;

				if (childItem.m_Func != NULL)
				{
					if (ImGui::MenuItem(childItem.m_Name.c_str()))
						childItem.m_Func(m_pObject, m_CurrentMenuType);
				}
				else if (ImGui::BeginMenu(childItem.m_Name.c_str()))
				{
					MenusRecursive(childItem);
					ImGui::EndMenu();
				}
			});
			ImGui::EndPopup();
		}
	}

	std::vector<std::string> ObjectMenu::DisectPath(const std::string& path)
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

	ObjectMenuItem* ObjectMenu::GetMenuItem(std::vector<ObjectMenuItem>& menuItems, const std::string& name, const ObjectMenuType& relevantMenus)
	{
		auto it = std::find_if(menuItems.begin(), menuItems.end(), [name](ObjectMenuItem& menuItem)
		{
			return menuItem.m_Name == name;
		});

		if (it != menuItems.end())
		{
			ObjectMenuItem* p = &*it;
			return p;
		}

		menuItems.push_back(ObjectMenuItem(name, relevantMenus));
		ObjectMenuItem* newItem = &menuItems[menuItems.size() - 1];
		return newItem;
	}

	void ObjectMenu::MenusRecursive(const ObjectMenuItem& menuItem)
	{
		std::for_each(menuItem.m_Children.begin(), menuItem.m_Children.end(), [&](const ObjectMenuItem& childItem)
		{
			if (childItem.m_RelevantMenus != T_Undefined && (childItem.m_RelevantMenus & m_CurrentMenuType) == 0) return;

			if (childItem.m_Func != NULL)
			{
				if (ImGui::MenuItem(childItem.m_Name.c_str()))
					childItem.m_Func(m_pObject, m_CurrentMenuType);
			}
			else if (ImGui::BeginMenu(childItem.m_Name.c_str()))
			{
				MenusRecursive(childItem);
				ImGui::EndMenu();
			}
		});
	}

	ObjectMenu::ObjectMenu() {}
	ObjectMenu::~ObjectMenu() {}

	ObjectMenuItem::ObjectMenuItem(const std::string& name, const ObjectMenuType& relevantMenus)
		: m_Name(name), m_RelevantMenus(relevantMenus), m_Func(NULL) {}
}

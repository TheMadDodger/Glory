#include "ObjectMenu.h"
#include <ResourceType.h>
#include <GScene.h>
#include <algorithm>
#include <imgui.h>
#include "Shortcuts.h"
#include "AssetManager.h"
#include <FileBrowserItem.h>

namespace Glory::Editor
{
	Object* ObjectMenu::m_pObject = nullptr;
	std::vector<ObjectMenuItemData> ObjectMenu::m_MenuItems;
	std::vector<ObjectMenuItem> ObjectMenu::m_BuiltMenu;
	ObjectMenuType ObjectMenu::m_CurrentMenuType = T_Undefined;
	bool ObjectMenu::m_Open = false;

	void ObjectMenu::Open(Object* pObject, ObjectMenuType forceMenuType)
	{
		m_pObject = pObject;
		m_CurrentMenuType = forceMenuType;
		m_Open = true;
	}

	void ObjectMenu::AddMenuItem(const std::string& path, std::function<void(Object*, const ObjectMenuType&)> func, const ObjectMenuTypeFlags& relevantMenus, std::string_view shortcut)
	{
		const size_t index = m_MenuItems.size();
		m_MenuItems.push_back(ObjectMenuItemData(path, func, (ObjectMenuType)relevantMenus));
		if (!shortcut.empty())
			Shortcuts::AddAction(shortcut.data(), std::bind(&ObjectMenuItemData::ShortcutCallback, m_MenuItems[index]));
	}

	void ObjectMenu::OnGUI()
	{
		if (m_Open)
		{
			BuildMenu();
			m_Open = false;
			if (m_BuiltMenu.size() <= 0) return;
			ImGui::OpenPopup("object_menu_popup");
		}

		if (ImGui::BeginPopup("object_menu_popup"))
		{
			ImGui::MenuItem("Object Menu", NULL, false, false);

			if (m_CurrentMenuType == T_Undefined)
			{
				ImGui::EndPopup();
				return;
			}

			std::for_each(m_BuiltMenu.begin(), m_BuiltMenu.end(), [&](const ObjectMenuItem& childItem)
			{
				if (childItem.m_ItemIndex != -1)
				{
					if (ImGui::MenuItem(childItem.m_Name.c_str()))
						m_MenuItems[childItem.m_ItemIndex].m_Func(m_pObject, m_CurrentMenuType);
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

		size_t forwardSlashIndex = currentPath.find("/");
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

	ObjectMenuItem* ObjectMenu::GetMenuItem(std::vector<ObjectMenuItem>& menuItems, const std::string& name)
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

		menuItems.push_back(ObjectMenuItem(name));
		ObjectMenuItem* newItem = &menuItems[menuItems.size() - 1];
		return newItem;
	}

	void ObjectMenu::MenusRecursive(const ObjectMenuItem& menuItem)
	{
		std::for_each(menuItem.m_Children.begin(), menuItem.m_Children.end(), [&](const ObjectMenuItem& childItem)
		{
			if (childItem.m_ItemIndex != -1)
			{
				if (ImGui::MenuItem(childItem.m_Name.c_str()))
					m_MenuItems[childItem.m_ItemIndex].m_Func(m_pObject, m_CurrentMenuType);
			}
			else if (ImGui::BeginMenu(childItem.m_Name.c_str()))
			{
				MenusRecursive(childItem);
				ImGui::EndMenu();
			}
		});
	}

	void ObjectMenu::Initialize()
	{
	}

	void ObjectMenu::Cleanup()
	{
		m_pObject = nullptr;
		m_MenuItems.clear();
		m_BuiltMenu.clear();
	}

	ObjectMenu::ObjectMenu() {}
	ObjectMenu::~ObjectMenu() {}

	void ObjectMenu::BuildMenu()
	{
		int counter = 0;
		m_BuiltMenu.clear();
		std::for_each(m_MenuItems.begin(), m_MenuItems.end(), [&](const ObjectMenuItemData& item)
		{
			if (item.m_RelevantMenus != T_Undefined && (item.m_RelevantMenus & m_CurrentMenuType) == 0)
			{
				++counter;
				return;
			}

			std::vector<std::string> items = DisectPath(item.m_Path);

			ObjectMenuItem* currentItem = GetMenuItem(m_BuiltMenu, items[0]);
			std::for_each(items.begin() + 1, items.end(), [&](const std::string& item)
			{
				currentItem = GetMenuItem(currentItem->m_Children, item);
			});
			currentItem->m_ItemIndex = counter;
			++counter;
		});
	}

	ObjectMenuItemData::ObjectMenuItemData(const std::string& path, std::function<void(Object*, const ObjectMenuType&)> func, const ObjectMenuType& relevantMenus)
		: m_Path(path), m_RelevantMenus(relevantMenus), m_Func(func) {}

	void ObjectMenuItemData::ShortcutCallback()
	{
		Object* pSelected = Selection::GetActiveObject();
		if (!pSelected)
		{
			const auto path = FileBrowserItem::GetHighlightedPath();
			if (path.empty() || !std::filesystem::exists(path))
			{
				m_Func(pSelected, T_Undefined);
				return;
			}

			if (std::filesystem::is_directory(path))
				m_Func(pSelected, T_Folder);
			else
				m_Func(pSelected, T_Resource);
			return;
		}
		for (size_t i = 0; i < pSelected->TypeCount(); i++)
		{
			std::type_index type = typeid(Object);
			if (!pSelected->GetType(i, type)) continue;
			if (type == typeid(Resource))
			{
				m_Func(pSelected, T_Resource);
				return;
			}
			if (type == typeid(SceneObject))
			{
				m_Func(pSelected, T_SceneObject);
				return;
			}
			if (type == typeid(GScene))
			{
				m_Func(pSelected, T_Scene);
				return;
			}
		}
		m_Func(pSelected, T_Undefined);
	}

	ObjectMenuItem::ObjectMenuItem(const std::string& name)
		: m_Name(name), m_ItemIndex(-1), m_Children() {}
}

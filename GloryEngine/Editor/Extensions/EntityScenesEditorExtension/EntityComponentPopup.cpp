#include "EntityComponentPopup.h"
#include <imgui.h>
#include <algorithm>
#include <Reflection.h>
#include <TypeFlags.h>

namespace Glory::Editor
{
	bool EntityComponentPopup::m_Open = false;
	uint32_t EntityComponentPopup::m_LastSelectedComponentTypeHash = 0;
	std::vector<ComponentMenuItem> EntityComponentPopup::m_MenuItems;

	void EntityComponentPopup::Open(GloryECS::EntityID entity, GloryECS::EntityRegistry* pRegistry)
	{
		m_Open = true;
		m_MenuItems.clear();

		for (size_t i = 0; i < GloryECS::ComponentTypes::ComponentCount(); ++i)
		{
			const GloryECS::ComponentType* componentType = GloryECS::ComponentTypes::GetComponentTypeAt(i);
			uint32_t typeHash = componentType->m_TypeHash;

			if (!componentType->m_AllowMultiple && pRegistry->HasComponent(entity, typeHash)) continue;

			const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(typeHash);

			if (pTypeData == nullptr) continue;

			std::vector<std::string> disectedPath = DisectPath(pTypeData->TypeName());
		
			if (disectedPath.size() <= 0) disectedPath.push_back(pTypeData->TypeName());
			if (disectedPath[0] == "")
				disectedPath[0] = pTypeData->TypeName();
		
			ComponentMenuItem* current = GetMenuItem(m_MenuItems, disectedPath[0]);
			for (size_t j = 1; j < disectedPath.size(); j++)
			{
				const std::string& subItem = disectedPath[j];
				current = GetMenuItem(current->m_Children, subItem);
			}
		
			current->m_ComponentTypeHash = typeHash;
		}
	}

	void EntityComponentPopup::OnGUI()
	{
		if (m_Open)
			ImGui::OpenPopup("EntityComponentPopup");
		m_Open = false;

		if (ImGui::BeginPopup("EntityComponentPopup"))
		{
			ImGui::Text("Add Component Menu");

			ImGui::InputText("##", m_FilterBuffer, 200);

			if (m_Filter != std::string(m_FilterBuffer))
			{
				m_Filter = std::string(m_FilterBuffer);
				RefreshFilter();
			}

			if (m_Filter.length() > 0)
			{
				for (size_t i = 0; i < m_FilteredItems.size(); i++)
				{
					ComponentMenuItem item = m_FilteredItems[i];
					std::string name = item.m_ItemName + "##" + std::to_string(item.m_ComponentTypeHash);
					if (ImGui::MenuItem(name.c_str()))
					{
						ComponentSelected(item);
					}
				}
			}
			else
			{
				DrawMenuItems(m_MenuItems);
			}

			ImGui::EndPopup();
		}
	}

	EntityComponentPopup::EntityComponentPopup() : m_FilterBuffer("")
	{
	}

	EntityComponentPopup::~EntityComponentPopup()
	{
	}

	void EntityComponentPopup::RefreshFilter()
	{
		m_FilteredItems.clear();
		if (m_Filter == "") return;

		size_t compCount = 0;

		std::vector<ComponentMenuItem*> pToCheck;
		size_t checkedCounter = 0;

		for (size_t i = 0; i < m_MenuItems.size(); i++)
		{
			pToCheck.push_back(&m_MenuItems[i]);
		}

		while (pToCheck.size() > checkedCounter)
		{
			for (size_t i = 0; i < pToCheck.size(); i++)
			{
				++checkedCounter;
				if (pToCheck[i]->m_Children.size() <= 0)
				{
					if (pToCheck[i]->m_ItemName.find(m_Filter) == std::string::npos) continue;
					m_FilteredItems.push_back(*pToCheck[i]);
					continue;
				}
				for (size_t j = 0; j < pToCheck[i]->m_Children.size(); j++)
				{
					pToCheck.push_back(&pToCheck[i]->m_Children[j]);
				}
			}
		}
	}

	ComponentMenuItem* EntityComponentPopup::GetMenuItem(std::vector<ComponentMenuItem>& children, const std::string& name)
	{
		auto it = std::find_if(children.begin(), children.end(), [&](const ComponentMenuItem& menuItem)
			{ return menuItem.m_ItemName == name; });

		if (it != children.end())
		{
			ComponentMenuItem* item = &*it;
			return item;
		}
		children.push_back(ComponentMenuItem(name));
		return &children[children.size() - 1];
	}

	void EntityComponentPopup::DrawMenuItems(std::vector<ComponentMenuItem>& menuItems)
	{
		std::for_each(menuItems.begin(), menuItems.end(), [&](ComponentMenuItem& menuItem)
		{
			std::string name = menuItem.m_ItemName + "##" + std::to_string(menuItem.m_ComponentTypeHash);
			if (menuItem.m_ComponentTypeHash != 0)
			{
				if (ImGui::MenuItem(name.c_str()))
				{
					ComponentSelected(menuItem);
				}
			}
			else
			{
				if (ImGui::BeginMenu(name.c_str()))
				{
					DrawMenuItems(menuItem.m_Children);
					ImGui::EndMenu();
				}
			}
		});
	}

	std::vector<std::string> EntityComponentPopup::DisectPath(const std::string& path)
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

	void EntityComponentPopup::ComponentSelected(const ComponentMenuItem& item)
	{
		m_LastSelectedComponentTypeHash = item.m_ComponentTypeHash;
		ImGui::CloseCurrentPopup();
	}

	uint32_t EntityComponentPopup::GetLastSelectedComponentTypeHash()
	{
		uint32_t selected = m_LastSelectedComponentTypeHash;
		m_LastSelectedComponentTypeHash = 0;
		return selected;
	}

	ComponentMenuItem::ComponentMenuItem(const std::string& itemName) : m_ItemName(itemName), m_ComponentTypeHash(0)
	{
	}

}

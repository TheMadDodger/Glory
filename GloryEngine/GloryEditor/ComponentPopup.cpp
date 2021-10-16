//#include "ComponentPopup.h"
//#include <TransformComponent.h>
//
//namespace Spartan::Editor
//{
//	bool ComponentPopup::m_Open = false;
//	BaseComponent* ComponentPopup::m_pLastSelectedComponent = nullptr;
//
//	void ComponentPopup::Open()
//	{
//		m_Open = true;
//	}
//
//	void ComponentPopup::OnGUI()
//	{
//		if (m_Open)
//			ImGui::OpenPopup("ComponentPopup");
//		m_Open = false;
//
//		if (m_MenuItems.size() == 0)
//			LoadComponents();
//
//		if (ImGui::BeginPopup("ComponentPopup"))
//		{
//			ImGui::Text("Add Component Menu");
//
//			ImGui::InputText("", m_FilterBuffer, 200);
//
//			if (m_Filter != std::string(m_FilterBuffer))
//			{
//				m_Filter = std::string(m_FilterBuffer);
//				RefreshFilter();
//			}
//
//			if (m_Filter.length() > 0)
//			{
//				for (size_t i = 0; i < m_pFilteredComponents.size(); i++)
//				{
//					BaseComponent* pComp = m_pFilteredComponents[i];
//					if (ImGui::MenuItem(pComp->GetName().c_str()))
//					{
//						ComponentSelected(pComp);
//					}
//				}
//			}
//			else
//			{
//				DrawMenuItems(m_MenuItems);
//			}
//
//			ImGui::EndPopup();
//		}
//	}
//
//	ComponentPopup::ComponentPopup() : m_FilterBuffer("")
//	{
//	}
//
//	ComponentPopup::~ComponentPopup()
//	{
//	}
//
//	void ComponentPopup::RefreshFilter()
//	{
//		m_pFilteredComponents.clear();
//		if (m_Filter == "") return;
//
//		size_t compCount = 0;
//		if (!SEObject::GetRegisteredClassCount<BaseComponent>(compCount)) return;
//		for (size_t i = 0; i < compCount; i++)
//		{
//			BaseComponent* pComp = SEObject::GetRegisteredClassAtIndex<BaseComponent>(i);
//			if (pComp->GetType() == typeid(TransformComponent)) continue;
//
//			if (pComp->GetName().find(m_Filter) == std::string::npos) continue;
//			m_pFilteredComponents.push_back(pComp);
//		}
//	}
//
//	void ComponentPopup::LoadComponents()
//	{
//		size_t compCount = 0;
//		if (!SEObject::GetRegisteredClassCount<BaseComponent>(compCount)) return;
//		for (size_t i = 0; i < compCount; i++)
//		{
//			BaseComponent* pComp = SEObject::GetRegisteredClassAtIndex<BaseComponent>(i);
//			if (pComp->GetType() == typeid(TransformComponent)) continue;
//			std::vector<std::string> disectedPath = DisectPath(pComp->GetComponentPath());
//
//			ComponentMenuItem* current = GetMenuItem(m_MenuItems, disectedPath[0]);
//			for (size_t j = 1; j < disectedPath.size(); j++)
//			{
//				const std::string& subItem = disectedPath[j];
//				current = GetMenuItem(current->m_Children, subItem);
//			}
//			current->m_pComponent = pComp;
//		}
//	}
//
//	ComponentMenuItem* ComponentPopup::GetMenuItem(std::vector<ComponentMenuItem>& children, const std::string& name)
//	{
//		auto it = std::find_if(children.begin(), children.end(), [&](const ComponentMenuItem& menuItem)
//			{ return menuItem.m_ItemName == name; });
//
//		if (it != children.end())
//		{
//			ComponentMenuItem* item = &*it;
//			return item;
//		}
//		children.push_back(ComponentMenuItem(name));
//		return &children[children.size() - 1];
//	}
//
//	void ComponentPopup::DrawMenuItems(std::vector<ComponentMenuItem>& menuItems)
//	{
//		std::for_each(menuItems.begin(), menuItems.end(), [&](ComponentMenuItem& menuItem)
//			{
//				if (menuItem.m_pComponent != nullptr)
//				{
//					if (ImGui::MenuItem(menuItem.m_ItemName.c_str()))
//					{
//						ComponentSelected(menuItem.m_pComponent);
//					}
//				}
//				else
//				{
//					if (ImGui::BeginMenu(menuItem.m_ItemName.c_str()))
//					{
//						DrawMenuItems(menuItem.m_Children);
//						ImGui::EndMenu();
//					}
//				}
//			});
//	}
//
//	std::vector<std::string> ComponentPopup::DisectPath(const std::string& path)
//	{
//		vector<string> result;
//		string currentPath = path;
//
//		int forwardSlashIndex = currentPath.find("/");
//		while (forwardSlashIndex != string::npos)
//		{
//			string item = currentPath.substr(0, forwardSlashIndex);
//			result.push_back(item);
//			currentPath = currentPath.substr(forwardSlashIndex + 1);
//			forwardSlashIndex = currentPath.find("/");
//		}
//		result.push_back(currentPath);
//		return result;
//	}
//
//	void ComponentPopup::ComponentSelected(BaseComponent* pComp)
//	{
//		m_pLastSelectedComponent = pComp;
//		ImGui::CloseCurrentPopup();
//	}
//
//	BaseComponent* ComponentPopup::GetLastSelectedComponent()
//	{
//		BaseComponent* pSelected = m_pLastSelectedComponent;
//		m_pLastSelectedComponent = nullptr;
//		return pSelected;
//	}
//
//	ComponentMenuItem::ComponentMenuItem(const std::string& itemName) : m_ItemName(itemName), m_pComponent(nullptr)
//	{
//	}
//}
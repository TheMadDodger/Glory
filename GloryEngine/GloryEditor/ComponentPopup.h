//#pragma once
//#include <BaseComponent.h>
//
//namespace Spartan::Editor
//{
//	struct ComponentMenuItem
//	{
//	public:
//		ComponentMenuItem(const std::string& itemName);
//
//		std::string m_ItemName;
//		BaseComponent* m_pComponent;
//		std::vector<ComponentMenuItem> m_Children;
//	};
//
//	class ComponentPopup
//	{
//	public:
//		static void Open();
//
//		void OnGUI();
//		static BaseComponent* GetLastSelectedComponent();
//
//	private:
//		friend class EditorApp;
//		ComponentPopup();
//		virtual ~ComponentPopup();
//
//		void RefreshFilter();
//		void LoadComponents();
//		ComponentMenuItem* GetMenuItem(std::vector<ComponentMenuItem>& children, const std::string& name);
//		void DrawMenuItems(std::vector<ComponentMenuItem>& menuItems);
//
//		std::vector<std::string> DisectPath(const std::string& path);
//
//		void ComponentSelected(BaseComponent* pComp);
//
//	private:
//		static bool m_Open;
//		char m_FilterBuffer[200];
//		std::string m_Filter;
//		std::vector<BaseComponent*> m_pFilteredComponents;
//		std::vector<ComponentMenuItem> m_MenuItems;
//		static BaseComponent* m_pLastSelectedComponent;
//	};
//}
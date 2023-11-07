#pragma once
#include <string>
#include <vector>
#include <EntityRegistry.h>

namespace Glory::Editor
{
	struct ComponentMenuItem
	{
	public:
		ComponentMenuItem(const std::string& itemName);

		std::string m_ItemName;
		uint32_t m_ComponentTypeHash;
		std::vector<ComponentMenuItem> m_Children;
	};

	class EntityComponentPopup
	{
	public:
		static void Open(Glory::Utils::ECS::EntityID entity, Glory::Utils::ECS::EntityRegistry* pRegistry);

		void OnGUI();
		static uint32_t GetLastSelectedComponentTypeHash();

	private:
		EntityComponentPopup();
		virtual ~EntityComponentPopup();

		void RefreshFilter();
		void DrawMenuItems(std::vector<ComponentMenuItem>& menuItems);

		static ComponentMenuItem* GetMenuItem(std::vector<ComponentMenuItem>& children, const std::string& name);
		static std::vector<std::string> DisectPath(const std::string& path);

		void ComponentSelected(const ComponentMenuItem& item);

	private:
		friend class EntitySceneObjectEditor;
		static bool m_Open;
		char m_FilterBuffer[200];
		std::string m_Filter;
		std::vector<ComponentMenuItem> m_FilteredItems;
		static std::vector<ComponentMenuItem> m_MenuItems;
		static uint32_t m_LastSelectedComponentTypeHash;
	};
}
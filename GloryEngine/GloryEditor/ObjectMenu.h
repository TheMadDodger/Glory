#pragma once
#include <Object.h>
#include <functional>

namespace Glory::Editor
{
	enum ObjectMenuType : unsigned int
	{
		T_Undefined = 0x00,
		T_Hierarchy = 0x01,
		T_SceneObject = 0x02,
		T_Scene = 0x04,
		T_SceneView = 0x08,
		T_Resource = 0x10,
		T_ContentBrowser = 0x20,
	};

	typedef unsigned int ObjectMenuTypeFlags;

	struct ObjectMenuItem
	{
	public:
		ObjectMenuItem(const std::string& name, const ObjectMenuType& relevantMenus);

		const std::string m_Name;
		const ObjectMenuType m_RelevantMenus;
		std::vector<ObjectMenuItem> m_Children;
		std::function<void(Object*, const ObjectMenuType&)> m_Func;
	};

	class ObjectMenu
	{
	public:
		static void Open(Object* pObject, ObjectMenuType forceMenuType);
		static void AddMenuItem(std::string path, std::function<void(Object*, const ObjectMenuType&)> func, const ObjectMenuTypeFlags& relevantMenus = ObjectMenuType::T_Undefined);

	private:
		static void OnGUI();
		static std::vector<std::string> DisectPath(const std::string& path);

		static ObjectMenuItem* GetMenuItem(std::vector<ObjectMenuItem>& menuItems, const std::string& name, const ObjectMenuType& relevantMenus);
		static void MenusRecursive(const ObjectMenuItem& menuItem);

	private:
		ObjectMenu();
		virtual ~ObjectMenu();

	private:
		friend class MainEditor;
		static Object* m_pObject;
		static ObjectMenuType m_CurrentMenuType;
		static std::vector<ObjectMenuItem> m_MenuItems;
		static bool m_Open;
	};
}

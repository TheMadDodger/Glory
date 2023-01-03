#pragma once
#include <Object.h>
#include <functional>
#include <Selection.h>
#include "GloryEditor.h"

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
		T_ModuleResource = 0x20,
		T_AnyResource = T_Resource | T_ModuleResource,
		T_ContentBrowser = 0x40,
		T_Folder = 0x80,
		T_ModuleFolder = 0x100,
		T_AnyFolder = T_Folder | T_ModuleFolder,
	};

	typedef unsigned int ObjectMenuTypeFlags;

	struct ObjectMenuItemData
	{
	public:
		ObjectMenuItemData(const std::string& path, std::function<void(Object*, const ObjectMenuType&)> func, const ObjectMenuType& relevantMenus);

		const std::string m_Path;
		const ObjectMenuType m_RelevantMenus;
		std::function<void(Object*, const ObjectMenuType&)> m_Func;

		void ShortcutCallback();
	};

	struct ObjectMenuItem
	{
	public:
		ObjectMenuItem(const std::string& name);

		const std::string m_Name;
		int m_ItemIndex;
		std::vector<ObjectMenuItem> m_Children;
	};

	class ObjectMenu
	{
	public:
		static GLORY_EDITOR_API void Open(Object* pObject, ObjectMenuType forceMenuType);
		static GLORY_EDITOR_API void AddMenuItem(const std::string& path, std::function<void(Object*, const ObjectMenuType&)> func, const ObjectMenuTypeFlags& relevantMenus, std::string_view shortcut = "");

	private:
		static void OnGUI();
		static std::vector<std::string> DisectPath(const std::string& path);

		static ObjectMenuItem* GetMenuItem(std::vector<ObjectMenuItem>& menuItems, const std::string& name);
		static void MenusRecursive(const ObjectMenuItem& menuItem);

		static void Initialize();
		static void Cleanup();

		static void BuildMenu();

	private:
		ObjectMenu();
		virtual ~ObjectMenu();


	private:
		friend class MainEditor;
		friend struct ObjectMenuItemData;
		static Object* m_pObject;
		static ObjectMenuType m_CurrentMenuType;
		static std::vector<ObjectMenuItemData> m_MenuItems;
		static std::vector<ObjectMenuItem> m_BuiltMenu;
		static bool m_Open;
	};
}

#pragma once
#include <vector>
#include <imgui.h>

namespace Glory::Editor
{
	class Editor;
	class PropertyDrawer;
	class Gizmos;

	class Editors
	{
	private:
		Editors();
		virtual ~Editors();

	private:
		friend class EditorContext;
		friend class Editor;

		std::vector<Editor*> m_pRegisteredEditors;
		std::vector<Editor*> m_pActiveEditors;
	};

	class EditorContext
	{
	public:
		static void SetContext(EditorContext* pContext);
		static EditorContext* GetContext();
		static EditorContext* CreateContext();
		static Gizmos* GetGizmos();

	private:
		EditorContext();
		virtual ~EditorContext();

		static void DestroyContext();
		static Editors* GetEditors();
		void Initialize();

	private:
		friend class EditorApplication;
		friend class PropertyDrawer;
		friend class Editor;
		static EditorContext* m_pContext;
		Editors* m_Editors;
		Gizmos* m_pGizmos;
		std::vector<PropertyDrawer*> m_PropertyDrawers;
		ImGuiContext* m_pIMGUIContext;
	};
}

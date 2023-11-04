#pragma once
#include <Editor.h>
#include <EntitySceneObject.h>
#include <EntityComponentObject.h>
#include "EntityComponentPopup.h"

namespace Glory::Editor
{
    class EntitySceneObjectEditor : public EditorTemplate<EntitySceneObjectEditor, EntitySceneObject>
    {
	public:
		EntitySceneObjectEditor();
		virtual ~EntitySceneObjectEditor();
		virtual bool OnGUI() override;

		void Refresh();

		template<typename T>
		static std::string_view GetComponentIcon()
		{
			return GetComponentIcon(ResourceType::GetHash<T>());
		}

		GLORY_EDITOR_API static std::string_view GetComponentIcon(uint32_t typeHash);

	private:
		void Initialize();
		bool NameGUI();
		bool ComponentGUI();

		static void DrawObjectNodeName(SceneObject* pObject, bool isPrefab);
		static bool SearchCompare(std::string_view search, SceneObject* pObject);

	private:
		friend class EntityScenesEditorExtension;
		std::vector<Editor*> m_pComponentEditors;
		bool m_Initialized;
		static const int MAXNAMESIZE = 250;
		char m_NameBuff[MAXNAMESIZE];
		bool m_AddingComponent;

		SceneObject* m_pObject;
		std::vector<EntityComponentObject*> m_pComponents;

		EntityComponentPopup m_ComponentPopup;
    };
}

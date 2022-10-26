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

	private:
		void Initialize();
		bool NameGUI();
		bool ComponentGUI();

	private:
		std::vector<Editor*> m_pComponentEditors;
		bool m_Initialized;
		static const int MAXNAMESIZE = 250;
		char m_NameBuff[MAXNAMESIZE];
		bool m_AddingComponent;

		SceneObject* m_pObject;
		std::vector<EntityComponentObject*> m_pComponents;

		//EntityComponentPopup m_ComponentPopup;

		size_t m_RightClickedComponentIndex = 0;
    };
}

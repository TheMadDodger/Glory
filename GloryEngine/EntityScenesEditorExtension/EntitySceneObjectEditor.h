#pragma once
#include <Editor.h>
#include <EntitySceneObject.h>
#include "EntityComponentObject.h"

namespace Glory::Editor
{
    class EntitySceneObjectEditor : public EditorTemplate<EntitySceneObjectEditor, EntitySceneObject>
    {
	public:
		EntitySceneObjectEditor();
		virtual ~EntitySceneObjectEditor();
		virtual void OnGUI() override;

	private:
		void Initialize();

	private:
		std::vector<Editor*> m_pComponentEditors;
		bool m_Initialized;
		static const int MAXNAMESIZE = 250;
		char m_NameBuff[MAXNAMESIZE];
		bool m_AddingComponent;

		std::vector<EntityComponentObject*> m_pComponents;
    };
}

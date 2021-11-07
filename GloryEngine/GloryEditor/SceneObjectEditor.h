#pragma once
#include "Editor.h"
#include <SceneObject.h>

namespace Glory::Editor
{
	class SceneObjectEditor : public EditorTemplate<SceneObjectEditor, SceneObject>
	{
	public:
		SceneObjectEditor();
		virtual ~SceneObjectEditor();
		virtual void OnGUI() override;

	private:
		void Initialize();

	private:
		std::vector<Editor*> m_pComponentEditors;
		bool m_Initialized;
		static const int MAXNAMESIZE = 250;
		char m_NameBuff[MAXNAMESIZE];
		bool m_AddingComponent;
	};
}
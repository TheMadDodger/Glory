#pragma once
#include "EditorWindow.h"
#include "Editor.h"

namespace Glory::Editor
{
	class InspectorWindow : public EditorWindowTemplate<InspectorWindow>
	{
	public:
		InspectorWindow();
		virtual ~InspectorWindow();

	private:
		virtual void OnGUI() override;
		void CreateEditor();

	private:
		//SEObject* m_pCurrentObject;
		//Editor* m_pEditor;

		bool m_Locked;
	};
}
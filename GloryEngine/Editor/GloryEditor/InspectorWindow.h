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

		void OnSelectionChange();

	private:
		Object* m_pCurrentObject;
		Editor* m_pEditor;
		UUID m_SelectionCallbackID;

		bool m_Locked;
	};
}
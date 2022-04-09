#include <imgui.h>
#include <AssetDatabase.h>
#include "InspectorWindow.h"
#include "Selection.h"
#include "Editor.h"

namespace Glory::Editor
{
	InspectorWindow::InspectorWindow() : EditorWindowTemplate("Inspector", 300.0f, 680.0f), m_Locked(false), m_pCurrentObject(nullptr), m_pEditor(nullptr)
	{

	}

	InspectorWindow::~InspectorWindow()
	{

	}

	void InspectorWindow::OnGUI()
	{
		Object* pSelectedObject = Selection::GetActiveObject();
		
		if (pSelectedObject != m_pCurrentObject && !m_Locked)
		{
			m_pCurrentObject = pSelectedObject;
			CreateEditor();
		}

		ImGui::Checkbox("Lock Inspector", &m_Locked);

		if (m_pEditor)
		{
			if (m_pEditor->OnGUI()) AssetDatabase::SetDirty(pSelectedObject);
		}
	}

	void InspectorWindow::CreateEditor()
	{
		if (!m_pCurrentObject)
		{
			m_pEditor = nullptr;
			return;
		}
		m_pEditor = Editor::CreateEditor(m_pCurrentObject);
	}
}
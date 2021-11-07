#include <algorithm>
#include <imgui.h>
#include "SceneObjectEditor.h"
//#include "BaseComponent.h"
#include "ComponentPopup.h"

namespace Glory::Editor
{
	SceneObjectEditor::SceneObjectEditor() : m_NameBuff(""), m_Initialized(false), m_AddingComponent(false)
	{
	}

	SceneObjectEditor::~SceneObjectEditor()
	{
	}

	void SceneObjectEditor::OnGUI()
	{
		if (!m_Initialized) Initialize();
		SceneObject* pObject = (SceneObject*)m_pTarget;

		const std::string& nameString = pObject->Name();
		const char* name = nameString.c_str();
		memcpy(m_NameBuff, name, nameString.length() + 1);
		m_NameBuff[nameString.length()] = '\0';

		ImGui::Text("Name");
		ImGui::SameLine();
		ImGui::InputText("##Name", m_NameBuff, MAXNAMESIZE);
		pObject->SetName(m_NameBuff);

		std::for_each(m_pComponentEditors.begin(), m_pComponentEditors.end(), [&](Editor* pEditor)
		{
			pEditor->OnGUI();
		});

		if (ImGui::Button("Add Component"))
		{
			//ComponentPopup::Open();
			//m_AddingComponent = true;
		}

		if (m_AddingComponent)
		{
			//BaseComponent* pToAddComp = ComponentPopup::GetLastSelectedComponent();
			//if (pToAddComp)
			//{
			//	pObject->AddComponent(pToAddComp);
			//	m_AddingComponent = false;
			//	Initialize();
			//}
		}
	}

	void SceneObjectEditor::Initialize()
	{
		m_pComponentEditors.clear();
		SceneObject* pObject = (SceneObject*)m_pTarget;
		//const vector<BaseComponent*>& pComponents = pGameObject->GetAllComponents();
		//std::for_each(pComponents.begin(), pComponents.end(), [&](BaseComponent* pComponent)
		//{
		//	Editor* pEditor = Editor::CreateEditor(pComponent);
		//	if (pEditor) this->m_pComponentEditors.push_back(pEditor);
		//});
		m_Initialized = true;
	}
}
//#include "GameObjectEditor.h"
//#include "BaseComponent.h"
//#include "ComponentPopup.h"
//
//namespace Spartan::Editor
//{
//	GameObjectEditor::GameObjectEditor() : m_Initialized(false), m_AddingComponent(false)
//	{
//	}
//
//	GameObjectEditor::~GameObjectEditor()
//	{
//	}
//
//	void GameObjectEditor::OnGUI()
//	{
//		if (!m_Initialized) Initialize();
//		GameObject* pObject = (GameObject*)m_pTarget;
//
//		const char* name = pObject->GetName();
//		memcpy(m_NameBuff, name, strlen(name));
//		ImGui::InputText("Object Name", m_NameBuff, MAXNAMESIZE);
//		pObject->SetName(m_NameBuff);
//
//		std::for_each(m_pComponentEditors.begin(), m_pComponentEditors.end(), [&](Editor* pEditor)
//			{
//				pEditor->OnGUI();
//			});
//
//		if (ImGui::Button("Add Component"))
//		{
//			ComponentPopup::Open();
//			m_AddingComponent = true;
//		}
//
//		if (m_AddingComponent)
//		{
//			BaseComponent* pToAddComp = ComponentPopup::GetLastSelectedComponent();
//			if (pToAddComp)
//			{
//				pObject->AddComponent(pToAddComp);
//				m_AddingComponent = false;
//				Initialize();
//			}
//		}
//	}
//
//	void GameObjectEditor::Initialize()
//	{
//		m_pComponentEditors.clear();
//		GameObject* pGameObject = (GameObject*)m_pTarget;
//		const vector<BaseComponent*>& pComponents = pGameObject->GetAllComponents();
//		std::for_each(pComponents.begin(), pComponents.end(), [&](BaseComponent* pComponent)
//			{
//				Editor* pEditor = Editor::CreateEditor(pComponent);
//				if (pEditor) this->m_pComponentEditors.push_back(pEditor);
//			});
//		m_Initialized = true;
//	}
//}
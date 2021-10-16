//#include "ComponentEditor.h"
//#include "SerializedProperty.h"
//#include "PropertyDrawer.h"
//
//namespace Spartan::Editor
//{
//	ComponentEditor::ComponentEditor()
//	{
//	}
//
//	ComponentEditor::~ComponentEditor()
//	{
//	}
//
//	void ComponentEditor::OnGUI()
//	{
//		BaseComponent* pComponent = (BaseComponent*)m_pTarget;
//		size_t id = Editor::GetID(this);
//
//		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
//		if (pComponent->GetSerializedPropertyCount() <= 0) pComponent->CollectProperties();
//
//		bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)id, node_flags, pComponent->GetName().c_str());
//		if (node_open)
//		{
//			for (size_t i = 0; i < pComponent->GetSerializedPropertyCount(); i++)
//			{
//				Serialization::SerializedProperty& prop = pComponent->GetSerializedProperty(i);
//				PropertyDrawer::DrawProperty(prop);
//			}
//			ImGui::TreePop();
//		}
//	}
//}
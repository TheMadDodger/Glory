//#include "ScriptableObjectEditor.h"
//#include "PropertyDrawer.h"
//
//namespace Spartan::Editor
//{
//	ScriptableObjectEditor::ScriptableObjectEditor()
//	{
//	}
//
//	ScriptableObjectEditor::~ScriptableObjectEditor()
//	{
//	}
//
//	void ScriptableObjectEditor::OnGUI()
//	{
//		Serialization::BaseAsset* pAsset = (Serialization::BaseAsset*)m_pTarget;
//		size_t id = Editor::GetID(this);
//
//		if (pAsset->GetSerializedPropertyCount() <= 0) pAsset->CollectProperties();
//		
//		for (size_t i = 0; i < pAsset->GetSerializedPropertyCount(); i++)
//		{
//			Serialization::SerializedProperty& prop = pAsset->GetSerializedProperty(i);
//			if (PropertyDrawer::DrawProperty(prop)) pAsset->SetDirty();
//		}
//	}
//}
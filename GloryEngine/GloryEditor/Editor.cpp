//#include "Editor.h"
//
//namespace Spartan::Editor
//{
//	std::vector<Editor*> Editor::m_pRegisteredEditors = std::vector<Editor*>();
//	std::vector<Editor*> Editor::m_pActiveEditors = std::vector<Editor*>();
//
//	Editor* Editor::CreateEditor(SEObject* pObject)
//	{
//		const std::type_info& baseType = pObject->GetBaseType();
//		const std::type_info& type = pObject->GetBaseType();
//
//		auto it = std::find_if(m_pRegisteredEditors.begin(), m_pRegisteredEditors.end(), [&](Editor* pEditor)
//			{
//				return pEditor->GetEditedType() == type;
//			});
//
//		if (it != m_pRegisteredEditors.end())
//		{
//			Editor* pEditor = *it;
//			Editor* newEditor = pEditor->Create();
//			newEditor->m_pTarget = pObject;
//			m_pActiveEditors.push_back(newEditor);
//			return newEditor;
//		}
//
//		auto it2 = std::find_if(m_pRegisteredEditors.begin(), m_pRegisteredEditors.end(), [&](Editor* pEditor)
//			{
//				return pEditor->GetEditedType() == baseType;
//			});
//
//		if (it2 == m_pRegisteredEditors.end()) return nullptr;
//
//		Editor* pEditor = *it;
//		Editor* newEditor = pEditor->Create();
//		newEditor->m_pTarget = pObject;
//		m_pActiveEditors.push_back(newEditor);
//		return newEditor;
//	}
//
//	size_t Editor::GetID(Editor* pEditor)
//	{
//		for (size_t i = 0; i < m_pActiveEditors.size(); i++)
//		{
//			if (m_pActiveEditors[i] == pEditor) return i;
//		}
//		return 0;
//	}
//
//	Editor::Editor()
//	{
//	}
//
//	Editor::~Editor()
//	{
//	}
//
//	void Editor::Cleanup()
//	{
//		std::for_each(m_pRegisteredEditors.begin(), m_pRegisteredEditors.end(), [](Editor* pEditor) {delete pEditor;});
//		std::for_each(m_pActiveEditors.begin(), m_pActiveEditors.end(), [](Editor* pEditor) {delete pEditor;});
//		m_pRegisteredEditors.clear();
//		m_pActiveEditors.clear();
//	}
//
//	template<class TEditor, class TObject>
//	inline void EditorTemplate<TEditor, TObject>::CompilerTest()
//	{
//		Editor* pObject = new TEditor();
//		SEObject* pObject = new TObject();
//	}
//}
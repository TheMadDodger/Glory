//#include "Selection.h"
//
//namespace Spartan::Editor
//{
//	vector<SEObject*> Selection::m_pSelectedObjects = vector<SEObject*>();
//
//	void Selection::SetActiveObject(SEObject* pObject)
//	{
//		m_pSelectedObjects.clear();
//		m_pSelectedObjects.push_back(pObject);
//	}
//
//	SEObject* Selection::GetActiveObject()
//	{
//		if (m_pSelectedObjects.size() <= 0) return nullptr;
//		return m_pSelectedObjects[0];
//	}
//
//	void Selection::Clear()
//	{
//		m_pSelectedObjects.clear();
//	}
//
//	Selection::Selection()
//	{
//	}
//
//	Selection::~Selection()
//	{
//	}
//}
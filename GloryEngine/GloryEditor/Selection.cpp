#include "Selection.h"

namespace Glory::Editor
{
	std::vector<Object*> Selection::m_pSelectedObjects = std::vector<Object*>();

	GLORY_EDITOR_API void Selection::SetActiveObject(Object* pObject)
	{
		m_pSelectedObjects.clear();
		m_pSelectedObjects.push_back(pObject);
	}

	GLORY_EDITOR_API Object* Selection::GetActiveObject()
	{
		if (m_pSelectedObjects.size() <= 0) return nullptr;
		return m_pSelectedObjects[0];
	}

	GLORY_EDITOR_API void Selection::Clear()
	{
		m_pSelectedObjects.clear();
	}

	GLORY_EDITOR_API bool Selection::IsObjectSelected(Object* pObject)
	{
		auto it = std::find(m_pSelectedObjects.begin(), m_pSelectedObjects.end(), pObject);
		return it != m_pSelectedObjects.end();
	}

	Selection::Selection()
	{
	}

	Selection::~Selection()
	{
	}
}
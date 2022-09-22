#include <algorithm>
#include "Selection.h"
#include "Undo.h"
#include "SelectAction.h"
#include "DeselectAction.h"

namespace Glory::Editor
{
	std::vector<Object*> Selection::m_pSelectedObjects = std::vector<Object*>();

	GLORY_EDITOR_API void Selection::SetActiveObject(Object* pObject)
	{
		Undo::StartRecord("Selection Changed");
		for (size_t i = 0; i < m_pSelectedObjects.size(); i++)
		{
			Undo::AddAction(new DeselectAction(m_pSelectedObjects[i]->GetUUID()));
		}

		m_pSelectedObjects.clear();

		if (pObject == nullptr)
		{
			Undo::StopRecord();
			return;
		}

		Undo::AddAction(new SelectAction(pObject->GetUUID()));
		m_pSelectedObjects.push_back(pObject);
		Undo::StopRecord();
	}

	GLORY_EDITOR_API Object* Selection::GetActiveObject()
	{
		if (m_pSelectedObjects.size() <= 0) return nullptr;
		return m_pSelectedObjects[0];
	}

	GLORY_EDITOR_API void Selection::Clear()
	{
		Undo::StartRecord("Selection Changed");
		for (size_t i = 0; i < m_pSelectedObjects.size(); i++)
		{
			Undo::AddAction(new DeselectAction(m_pSelectedObjects[i]->GetUUID()));
		}
		m_pSelectedObjects.clear();
		Undo::StopRecord();
	}

	GLORY_EDITOR_API bool Selection::IsObjectSelected(Object* pObject)
	{
		auto it = std::find(m_pSelectedObjects.begin(), m_pSelectedObjects.end(), pObject);
		return it != m_pSelectedObjects.end();
	}

	void Selection::AddObjectToSelection(Object* pObject)
	{
		if (pObject == nullptr) return;
		Undo::StartRecord("Selection Changed");
		Undo::AddAction(new SelectAction(pObject->GetUUID()));
		m_pSelectedObjects.push_back(pObject);
		Undo::StopRecord();
	}

	void Selection::RemoveObjectFromSelection(Object* pObject)
	{
		if (pObject == nullptr) return;
		if (m_pSelectedObjects.size() <= 0) return;
		auto it = std::find(m_pSelectedObjects.begin(), m_pSelectedObjects.end(), pObject);
		if (it == m_pSelectedObjects.end()) return;

		Undo::StartRecord("Selection Changed");
		Undo::AddAction(new DeselectAction(pObject->GetUUID()));
		m_pSelectedObjects.erase(it);
		Undo::StopRecord();
	}

	void Selection::AddObjectToSelection(UUID objectID)
	{
		Object* pObject = Object::FindObject(objectID);
		AddObjectToSelection(pObject);
	}

	void Selection::RemoveObjectFromSelection(UUID objectID)
	{
		Object* pObject = Object::FindObject(objectID);
		RemoveObjectFromSelection(pObject);
	}

	Selection::Selection()
	{
	}

	Selection::~Selection()
	{
	}
}
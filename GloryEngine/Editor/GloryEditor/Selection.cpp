#include <algorithm>
#include "Selection.h"
#include "Undo.h"
#include "SelectAction.h"
#include "DeselectAction.h"

namespace Glory::Editor
{
	std::vector<Object*> Selection::m_pSelectedObjects = std::vector<Object*>();
	std::map<UUID, std::function<void()>> Selection::m_SelectionChangeCallback;

	void Selection::SetActiveObject(Object* pObject)
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
			TriggerSelectionChangeCallback();
			return;
		}

		Undo::AddAction(new SelectAction(pObject->GetUUID()));
		m_pSelectedObjects.push_back(pObject);
		Undo::StopRecord();
		TriggerSelectionChangeCallback();
	}

	Object* Selection::GetActiveObject()
	{
		if (m_pSelectedObjects.size() <= 0) return nullptr;
		return m_pSelectedObjects[0];
	}

	void Selection::Clear()
	{
		Undo::StartRecord("Selection Changed");
		for (size_t i = 0; i < m_pSelectedObjects.size(); i++)
		{
			Undo::AddAction(new DeselectAction(m_pSelectedObjects[i]->GetUUID()));
		}
		m_pSelectedObjects.clear();
		Undo::StopRecord();
		TriggerSelectionChangeCallback();
	}

	bool Selection::IsObjectSelected(Object* pObject)
	{
		auto it = std::find(m_pSelectedObjects.begin(), m_pSelectedObjects.end(), pObject);
		return it != m_pSelectedObjects.end();
	}

	UUID Selection::SubscribeToSelectionChange(std::function<void()> callback)
	{
		UUID uuid = UUID();
		m_SelectionChangeCallback.emplace(uuid, callback);
		return uuid;
	}

	void Selection::UnsubscribeToSelectionChange(UUID uuid)
	{
		m_SelectionChangeCallback.erase(uuid);
	}

	void Selection::TriggerSelectionChangeCallback()
	{
		for (auto it = m_SelectionChangeCallback.begin(); it != m_SelectionChangeCallback.end(); ++it)
		{
			it->second();
		}
	}

	void Selection::AddObjectToSelection(Object* pObject)
	{
		if (pObject == nullptr) return;
		Undo::StartRecord("Selection Changed");
		Undo::AddAction(new SelectAction(pObject->GetUUID()));
		m_pSelectedObjects.push_back(pObject);
		Undo::StopRecord();
		TriggerSelectionChangeCallback();
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
		TriggerSelectionChangeCallback();
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
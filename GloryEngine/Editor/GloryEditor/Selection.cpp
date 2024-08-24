#include "Selection.h"
#include "Undo.h"
#include "SelectionChangedAction.h"
#include "EditorApplication.h"

#include <ObjectManager.h>

#include <algorithm>

namespace Glory::Editor
{
	std::vector<Object*> Selection::m_pSelectedObjects = std::vector<Object*>();
	std::map<UUID, std::function<void()>> Selection::m_SelectionChangeCallback;

	void Selection::SetActiveObject(Object* pObject)
	{
		const bool wasRecording = Undo::IsRecording();
		if (!wasRecording)
			Undo::StartRecord("Selection Changed");
		std::vector<UUID> oldSelection = CreateOldSelectionArray();
		m_pSelectedObjects.clear();

		if (pObject == nullptr)
		{
			Undo::AddAction<SelectionChangedAction>(std::move(oldSelection));
			Undo::StopRecord();
			TriggerSelectionChangeCallback();
			return;
		}

		m_pSelectedObjects.push_back(pObject);
		Undo::AddAction<SelectionChangedAction>(std::move(oldSelection));
		if (!wasRecording)
			Undo::StopRecord();
		TriggerSelectionChangeCallback();
	}

	void Selection::SetActiveObjectNoUndo(Object* pObject)
	{
		m_pSelectedObjects.clear();

		if (pObject == nullptr)
		{
			TriggerSelectionChangeCallback();
			return;
		}
		m_pSelectedObjects.push_back(pObject);
	}

	Object* Selection::GetActiveObject()
	{
		if (m_pSelectedObjects.size() <= 0) return nullptr;
		return m_pSelectedObjects[0];
	}

	void Selection::Clear()
	{
		const bool wasRecording = Undo::IsRecording();
		if (!wasRecording)
			Undo::StartRecord("Selection Changed");
		std::vector<UUID> oldSelection = CreateOldSelectionArray();
		m_pSelectedObjects.clear();
		Undo::AddAction<SelectionChangedAction>(std::move(oldSelection));
		if (!wasRecording)
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

	size_t Selection::SelectionCount()
	{
		return m_pSelectedObjects.size();
	}

	Object* Selection::GetSelectedObject(size_t index)
	{
		return m_pSelectedObjects[index];
	}

	void Selection::TriggerSelectionChangeCallback()
	{
		for (auto it = m_SelectionChangeCallback.begin(); it != m_SelectionChangeCallback.end(); ++it)
		{
			it->second();
		}
	}

	std::vector<UUID> Selection::CreateOldSelectionArray()
	{
		std::vector<UUID> selection;
		for (size_t i = 0; i < m_pSelectedObjects.size(); ++i)
		{
			selection.push_back(m_pSelectedObjects[i]->GetUUID());
		}
		return selection;
	}

	void Selection::AddObjectToSelection(Object* pObject)
	{
		if (pObject == nullptr) return;
		const bool wasRecording = Undo::IsRecording();
		if (!wasRecording)
			Undo::StartRecord("Selection Changed");
		std::vector<UUID> oldSelection = CreateOldSelectionArray();
		m_pSelectedObjects.push_back(pObject);
		Undo::AddAction<SelectionChangedAction>(std::move(oldSelection));
		if (!wasRecording)
			Undo::StopRecord();
		TriggerSelectionChangeCallback();
	}

	void Selection::RemoveObjectFromSelection(Object* pObject)
	{
		if (pObject == nullptr) return;
		if (m_pSelectedObjects.size() <= 0) return;
		auto it = std::find(m_pSelectedObjects.begin(), m_pSelectedObjects.end(), pObject);
		if (it == m_pSelectedObjects.end()) return;

		const bool wasRecording = Undo::IsRecording();
		if (!wasRecording)
			Undo::StartRecord("Selection Changed");
		std::vector<UUID> oldSelection = CreateOldSelectionArray();
		m_pSelectedObjects.erase(it);
		Undo::AddAction<SelectionChangedAction>(std::move(oldSelection));
		if (!wasRecording)
			Undo::StopRecord();
		TriggerSelectionChangeCallback();
	}

	void Selection::AddObjectToSelection(UUID objectID)
	{
		Object* pObject = EditorApplication::GetInstance()->GetEngine()->GetObjectManager().Find(objectID);
		AddObjectToSelection(pObject);
	}

	void Selection::RemoveObjectFromSelection(UUID objectID)
	{
		Object* pObject = EditorApplication::GetInstance()->GetEngine()->GetObjectManager().Find(objectID);
		RemoveObjectFromSelection(pObject);
	}

	Selection::Selection()
	{
	}

	Selection::~Selection()
	{
	}
}
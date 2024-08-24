#include "SelectionChangedAction.h"
#include "Selection.h"

namespace Glory::Editor
{
	SelectionChangedAction::SelectionChangedAction(std::vector<UUID>&& oldSelection)
		: m_OldSelection(std::move(oldSelection))
	{
		for (size_t i = 0; i < Selection::SelectionCount(); ++i)
		{
			Object* pObject = Selection::GetSelectedObject(i);
			m_NewSelection.push_back(pObject->GetUUID());
		}
	}

	SelectionChangedAction::~SelectionChangedAction()
	{
	}

	void SelectionChangedAction::OnUndo(const ActionRecord& actionRecord)
	{
		Selection::Clear();
		for (size_t i = 0; i < m_OldSelection.size(); ++i)
		{
			Selection::AddObjectToSelection(m_OldSelection[i]);
		}
	}

	void SelectionChangedAction::OnRedo(const ActionRecord& actionRecord)
	{
		Selection::Clear();
		for (size_t i = 0; i < m_NewSelection.size(); ++i)
		{
			Selection::AddObjectToSelection(m_NewSelection[i]);
		}
	}
}

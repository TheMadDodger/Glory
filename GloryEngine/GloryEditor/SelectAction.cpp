#include "SelectAction.h"
#include "Selection.h"

namespace Glory::Editor
{
	SelectAction::SelectAction(UUID selectedObjectID) : m_SelectedObjectID(selectedObjectID)
	{
	}

	SelectAction::~SelectAction()
	{
	}

	void SelectAction::OnUndo(const ActionRecord& actionRecord)
	{
		Selection::RemoveObjectFromSelection(m_SelectedObjectID);
	}

	void SelectAction::OnRedo(const ActionRecord& actionRecord)
	{
		Selection::AddObjectToSelection(m_SelectedObjectID);
	}
}

#include "DeselectAction.h"
#include "Selection.h"

namespace Glory::Editor
{
	DeselectAction::DeselectAction(UUID deselectedObjectID) : m_DeselectedObjectID(deselectedObjectID)
	{
	}

	DeselectAction::~DeselectAction()
	{
	}

	void DeselectAction::OnUndo(const ActionRecord& actionRecord)
	{
		Selection::AddObjectToSelection(m_DeselectedObjectID);
	}

	void DeselectAction::OnRedo(const ActionRecord& actionRecord)
	{
		Selection::RemoveObjectFromSelection(m_DeselectedObjectID);
	}
}

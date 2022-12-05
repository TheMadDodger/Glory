#pragma once
#include "Undo.h"

namespace Glory::Editor
{
	class SetParentAction : public IAction
	{
	public:
		SetParentAction(UUID oldParent, UUID newParent);
		virtual ~SetParentAction() = default;

	private:
		void OnUndo(const ActionRecord& actionRecord) override;
		void OnRedo(const ActionRecord& actionRecord) override;

	private:
		UUID m_OldParent;
		UUID m_NewParent;
	};
}

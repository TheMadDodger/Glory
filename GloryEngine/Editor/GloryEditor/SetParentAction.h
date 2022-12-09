#pragma once
#include "Undo.h"

namespace Glory::Editor
{
	class SetParentAction : public IAction
	{
	public:
		SetParentAction(UUID oldParent, UUID newParent, size_t siblingIndex);
		virtual ~SetParentAction() = default;

	private:
		void OnUndo(const ActionRecord& actionRecord) override;
		void OnRedo(const ActionRecord& actionRecord) override;

	private:
		const UUID m_OldParent;
		const UUID m_NewParent;
		const size_t m_SiblingIndex;
	};
}

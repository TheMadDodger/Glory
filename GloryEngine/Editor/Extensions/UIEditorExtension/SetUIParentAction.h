#pragma once
#include <Undo.h>

#include <EntityID.h>

namespace Glory
{
	class Engine;
	class UIDocument;
}

namespace Glory::Editor
{
	class SetUIParentAction : public IAction
	{
	public:
		SetUIParentAction(UUID toReParent, UUID oldParent, UUID newParent, size_t oldSiblingIndex, size_t newSiblingIndex);
		virtual ~SetUIParentAction() = default;

		static void StoreDocumentState(Engine* pEngine, UIDocument* pDocument, Utils::NodeValueRef entities);

	private:
		void OnUndo(const ActionRecord& actionRecord) override;
		void OnRedo(const ActionRecord& actionRecord) override;

	private:
		const UUID m_ToReparent;
		const UUID m_OldParent;
		const UUID m_NewParent;
		const size_t m_OldSiblingIndex;
		const size_t m_NewSiblingIndex;
	};
}

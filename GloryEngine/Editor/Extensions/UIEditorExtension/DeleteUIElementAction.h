#pragma once
#include <Undo.h>

#include <NodeRef.h>

namespace Glory
{
	class IEngine;
	class UIDocument;
}

namespace Glory::Editor
{
	class DeleteUIElementAction : public IAction
	{
	public:
		DeleteUIElementAction(IEngine* pEngine, UUID uuid, UIDocument* pDocument, size_t siblingIndex);
		virtual ~DeleteUIElementAction() = default;

		static void DeleteElement(IEngine* pEngine, UIDocument* pDocument, Utils::YAMLFileRef& file, UUID uuid);

	private:
		void OnUndo(const ActionRecord& actionRecord) override;
		void OnRedo(const ActionRecord& actionRecord) override;

	private:
		Utils::InMemoryYAML m_SerializedObject;
		const UUID m_ID;
		const size_t m_SiblingIndex;
	};
}

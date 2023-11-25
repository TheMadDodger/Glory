#pragma once
#include "Undo.h"
#include "EntitySceneObjectEditor.h"

namespace Glory::Editor
{
	class AddComponentAction : public IAction
	{
	public:
		AddComponentAction(uint32_t typeHash, UUID uuid, size_t componentIndex);
		virtual ~AddComponentAction();

	private:
		virtual void OnUndo(const ActionRecord& actionRecord);
		virtual void OnRedo(const ActionRecord& actionRecord);

	private:
		const uint32_t m_ComponentTypeHash;
		const UUID m_ComponentUUID;
		const size_t m_ComponentIndex;
	};
}

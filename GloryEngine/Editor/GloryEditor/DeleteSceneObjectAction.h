#pragma once
#include "Undo.h"

#include <EntityID.h>

namespace Glory
{
	class GScene;
namespace Editor
{
	class DeleteSceneObjectAction : public IAction
	{
	public:
		DeleteSceneObjectAction(GScene* pScene, Utils::ECS::EntityID deletedEntity);
		virtual ~DeleteSceneObjectAction();

	private:
		virtual void OnUndo(const ActionRecord& actionRecord);
		virtual void OnRedo(const ActionRecord& actionRecord);

	private:
		Utils::InMemoryYAML m_SerializedObject;
		UUID m_OriginalSceneUUID;
		bool m_WasSelected;
	};
}
}

#pragma once
#include "Undo.h"
#include <SceneObject.h>
#include <Serializer.h>

namespace Glory::Editor
{
	class DeleteSceneObjectAction : public IAction
	{
	public:
		DeleteSceneObjectAction(SceneObject* pDeletedObject);
		virtual ~DeleteSceneObjectAction();

	private:
		virtual void OnUndo(const ActionRecord& actionRecord);
		virtual void OnRedo(const ActionRecord& actionRecord);

		void SerializeRecursive(SceneObject* pDeletedObject, YAML::Emitter& out);

	private:
		std::string m_SerializedObject;
		UUID m_OriginalSceneUUID;
	};
}

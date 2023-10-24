#include "EnableObjectAction.h"

#include <Engine.h>
#include <Game.h>

namespace Glory::Editor
{
	EnableObjectAction::EnableObjectAction(bool active)
		: m_Active(active)
	{
	}

	EnableObjectAction::~EnableObjectAction()
	{
	}

	void EnableObjectAction::OnUndo(const ActionRecord& actionRecord)
	{
		SceneObject* pObject = (SceneObject*)Object::FindObject(actionRecord.ObjectID);
		if (pObject == nullptr) return;
		pObject->SetActive(!m_Active);
	}

	void EnableObjectAction::OnRedo(const ActionRecord& actionRecord)
	{
		SceneObject* pObject = (SceneObject*)Object::FindObject(actionRecord.ObjectID);
		if (pObject == nullptr) return;
		pObject->SetActive(m_Active);
	}
}
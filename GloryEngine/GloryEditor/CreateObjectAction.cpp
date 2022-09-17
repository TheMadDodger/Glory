#include "CreateObjectAction.h"
#include <GScene.h>

namespace Glory::Editor
{
	CreateObjectAction::CreateObjectAction(SceneObject* pSceneObject) : m_pSceneObject(pSceneObject), m_pScene(nullptr), m_ObjectID()
	{

	}

	CreateObjectAction::~CreateObjectAction()
	{
	}

	void CreateObjectAction::OnUndo(const ActionRecord& actionRecord)
	{
		m_ObjectID = m_pSceneObject->GetUUID();
		m_pScene = m_pSceneObject->GetScene();
		m_pSceneObject->GetScene()->DeleteObject(m_pSceneObject);
		m_pSceneObject = nullptr;
	}

	void CreateObjectAction::OnRedo(const ActionRecord& actionRecord)
	{
		m_pSceneObject = m_pScene->CreateEmptyObject("Empty Object", m_ObjectID);
	}
}
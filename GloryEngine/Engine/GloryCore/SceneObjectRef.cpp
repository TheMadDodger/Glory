#include "SceneObjectRef.h"
#include "Engine.h"

namespace Glory
{
	SceneObjectRef::SceneObjectRef()
		: m_SceneUUID(0), m_ObjectUUID(0)
	{
	}

	SceneObjectRef::SceneObjectRef(UUID sceneUUID, UUID objectUUID)
		: m_SceneUUID(sceneUUID), m_ObjectUUID(objectUUID)
	{
	}

	SceneObjectRef::~SceneObjectRef()
	{
	}

	const UUID SceneObjectRef::SceneUUID() const
	{
		return m_SceneUUID;
	}

	UUID* SceneObjectRef::SceneUUIDMember()
	{
		return &m_SceneUUID;
	}

	const UUID SceneObjectRef::ObjectUUID() const
	{
		return m_ObjectUUID;
	}

	UUID* SceneObjectRef::ObjectUUIDMember()
	{
		return &m_ObjectUUID;
	}

	SceneObject* SceneObjectRef::GetObject()
	{
		/* FIXME: This is kinda slow */
		GScene* pScene = Game::GetGame().GetEngine()->GetScenesModule()->GetOpenScene(m_SceneUUID);
		if (!pScene) return nullptr;
		return pScene->FindSceneObject(m_ObjectUUID);
	}

	bool SceneObjectRef::operator==(const SceneObjectRef& other) const
	{
		return m_ObjectUUID == other.m_ObjectUUID && m_SceneUUID == other.m_SceneUUID;
	}

	bool SceneObjectRef::operator==(const SceneObjectRef&& other) const
	{
		return m_ObjectUUID == other.m_ObjectUUID && m_SceneUUID == other.m_SceneUUID;
	}
}

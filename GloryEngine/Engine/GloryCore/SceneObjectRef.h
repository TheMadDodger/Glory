#pragma once
#include "SceneObject.h"

namespace Glory
{
	class SceneObjectRef
	{
	public:
		SceneObjectRef();
		SceneObjectRef(UUID sceneUUID, UUID objectUUID);
		virtual ~SceneObjectRef();

		const UUID SceneUUID() const;
		UUID* SceneUUIDMember();
		const UUID ObjectUUID() const;
		UUID* ObjectUUIDMember();
		SceneObject* GetObject();

		bool operator==(const SceneObjectRef& other) const;
		bool operator==(const SceneObjectRef&& other) const;

	protected:
		REFLECTABLE(SceneObjectRef, (UUID)(m_SceneUUID), (UUID)(m_ObjectUUID))
	};
}

#pragma once
#include <GScene.h>
#include "GloryMono.h"

namespace Glory
{
	class MonoSceneObjectManager
	{
	public:
		GLORY_API MonoSceneObjectManager(GScene* pScene) : m_pScene(pScene) {}
		GLORY_API virtual ~MonoSceneObjectManager();

		GLORY_API MonoObject* GetMonoSceneObject(UUID objectID);
		GLORY_API void DestroyMonoSceneObject(UUID objectID);

	protected:
		MonoObject* GetMonoSceneObject_Internal(UUID objectID);
		void DestroyMonoSceneObject_Internal(MonoObject* pMonoObject);

	protected:
		GScene* m_pScene;

	private:
		std::map<UUID, MonoObject*> m_ObjectsCache;
	};
}

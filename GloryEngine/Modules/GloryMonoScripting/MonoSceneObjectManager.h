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

		GLORY_API MonoObject* GetSceneObject(UUID objectID);
		GLORY_API void DestroySceneObject(UUID objectID);

	protected:
		MonoObject* GetSceneObject_Internal(UUID objectID);
		void DestroySceneObject_Internal(MonoObject* pMonoObject);

	protected:
		GScene* m_pScene;

	private:
		std::map<UUID, MonoObject*> m_ObjectsCache;
	};
}

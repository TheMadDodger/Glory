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

		GLORY_API MonoObject* GetSceneObject(SceneObject* pObject);
		GLORY_API void DestroySceneObject(SceneObject* pObject);

	protected:
		MonoObject* GetSceneObject_Internal(SceneObject* pObject);
		void DestroySceneObject_Internal(MonoObject* pMonoObject);

	protected:
		GScene* m_pScene;

	private:
		std::map<SceneObject*, MonoObject*> m_ObjectsCache;
	};
}

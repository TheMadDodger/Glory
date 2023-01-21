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
		virtual MonoObject* GetSceneObject_Impl(SceneObject* pObject) = 0;
		virtual void DestroySceneObject_Impl(MonoObject* pMonoObject) = 0;

	protected:
		GScene* m_pScene;

	private:
		std::map<SceneObject*, MonoObject*> m_ObjectsCache;
	};
}

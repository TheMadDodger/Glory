#pragma once
#include <GScene.h>
#include "GloryMono.h"

namespace Glory
{
	class MonoSceneObjectManager
	{
	public:
		MonoSceneObjectManager(GScene* pScene) : m_pScene(pScene) {}

		virtual SceneObject* GetSceneObject(MonoObject* pMonoObject) = 0;
		virtual void DestroySceneObject(MonoClass* pClass, Object* pObject) = 0;

	protected:
		GScene* m_pScene;
	};
}

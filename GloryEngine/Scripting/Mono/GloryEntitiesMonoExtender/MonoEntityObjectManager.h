#pragma once
#include <MonoSceneObjectManager.h>

namespace Glory
{
	class MonoEntityObjectManager : public MonoSceneObjectManager
	{
	public:
		MonoEntityObjectManager(GScene* pScene) : MonoSceneObjectManager(pScene) {}

		SceneObject* GetSceneObject(MonoObject* pMonoObject) override;
		void DestroySceneObject(MonoClass* pClass, Object* pObject) override;
	};
}

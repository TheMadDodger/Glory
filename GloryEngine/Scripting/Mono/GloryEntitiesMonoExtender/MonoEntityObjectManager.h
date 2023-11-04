#pragma once
#include <MonoSceneObjectManager.h>
#include <GloryMono.h>

namespace Glory
{
	class MonoEntityObjectManager : public MonoSceneObjectManager
	{
	public:
		MonoEntityObjectManager(GScene* pScene);
		virtual ~MonoEntityObjectManager();

	private:
		MonoObject* GetSceneObject_Impl(SceneObject* pObject) override;
		void DestroySceneObject_Impl(MonoObject* pMonoObject) override;
	};
}

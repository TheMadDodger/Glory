#pragma once
#include <functional>
#include <Glory.h>
#include "GloryMono.h"
#include "MonoSceneObjectManager.h"

#define BINDER(name, ret, ...) \
static std::function<ret(__VA_ARGS__)> m_##name##Impl; \
static GLORY_API void Bind##name(std::function<ret(__VA_ARGS__)> f);

namespace Glory
{
	class MonoSceneManager
	{
	public:
		static GLORY_API MonoObject* GetSceneObject(GScene* pScene);
		static GLORY_API MonoSceneObjectManager* GetSceneObjectManager(GScene* pScene);
		static GLORY_API void DestroyScene(GScene* pScene);
		static GLORY_API void UnbindImplementation();

		template<class T>
		static void BindImplemetation()
		{
			CheckBound();
			BindGetSceneObject(T::GetSceneObject);
			BindGetSceneObjectManager(T::GetSceneObjectManager);
		}

	private:
		static void Cleanup();
		static GLORY_API void CheckBound();

	private:
		friend class MonoManager;
		/* No instances allowed */
		MonoSceneManager() = delete;

	private:
		BINDER(GetSceneObject, MonoObject*, GScene*);
		BINDER(GetSceneObjectManager, MonoSceneObjectManager*, GScene*);

		static bool m_Bound;

		/* Cache */
		static std::map<GScene*, MonoObject*> m_SceneObjectCache;
		static std::map<GScene*, MonoSceneObjectManager*> m_SceneObjectManagers;
	};
}

/* Prevent macro usage outside of this header */
#undef BINDER
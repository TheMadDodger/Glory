#pragma once
#include "GloryMono.h"
#include "MonoSceneObjectManager.h"

#include <functional>
#include <Glory.h>

#define GETTER(type, x) \
static GLORY_API type* x() \
{ \
	return m_p##x; \
}

namespace Glory
{
	class Assembly;

	class MonoSceneManager
	{
	public:
		static GLORY_API MonoObject* GetSceneObject(GScene* pScene);
		static GLORY_API MonoSceneObjectManager* GetSceneObjectManager(GScene* pScene);
		static GLORY_API void DestroyScene(GScene* pScene);

		GETTER(MonoClass, EntitySceneClass);
		GETTER(MonoClass, EntitySceneObjectClass);
		GETTER(MonoMethod, EntitySceneConstructor);
		GETTER(MonoMethod, EntitySceneObjectConstructor);

	private:
		static void Initialize(Assembly* pAssembly);
		static void Cleanup();

	private:
		friend class MonoManager;
		friend class CoreLibManager;
		/* No instances allowed */
		MonoSceneManager() = delete;

	private:
		static MonoObject* GetSceneObject_Internal(GScene* pScene);

		/* Cache */
		static std::map<GScene*, MonoObject*> m_SceneObjectCache;
		static std::map<GScene*, MonoSceneObjectManager*> m_SceneObjectManagers;

		static MonoClass* m_pEntitySceneClass;
		static MonoClass* m_pEntitySceneObjectClass;
		static MonoMethod* m_pEntitySceneConstructor;
		static MonoMethod* m_pEntitySceneObjectConstructor;
	};
}

#undef GETTER

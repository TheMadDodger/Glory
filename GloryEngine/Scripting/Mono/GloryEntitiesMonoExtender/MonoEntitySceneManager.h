#pragma once
#include "MonoEntityObjectManager.h"

#include <Glory.h>
#include <GloryMono.h>

#define GETTER(type, x) \
static GLORY_API type* x() \
{ \
	return m_p##x; \
}

namespace Glory
{
	class Assembly;

	class MonoEntitySceneManager
	{
	public:
		static GLORY_API MonoObject* GetSceneObject(GScene* pScene);
		static GLORY_API MonoSceneObjectManager* GetSceneObjectManager(GScene* pScene);

		GETTER(MonoClass, EntitySceneClass);
		GETTER(MonoClass, EntitySceneObjectClass);
		GETTER(MonoMethod, EntitySceneConstructor);
		GETTER(MonoMethod, EntitySceneObjectConstructor);

	private:
		static void Initialize(Assembly* pAssembly);
		static void Cleanup();

	private:
		friend class EntityLibManager;
		/* No instances allowed */
		MonoEntitySceneManager() = delete;

	private:
		static MonoClass* m_pEntitySceneClass;
		static MonoClass* m_pEntitySceneObjectClass;
		static MonoMethod* m_pEntitySceneConstructor;
		static MonoMethod* m_pEntitySceneObjectConstructor;
	};
}

#undef GETTER

#include "MonoSceneManager.h"
#include "Assembly.h"

#include <Debug.h>

#define GET_CLASS(name, out) \
out = mono_class_from_name(pAssembly->GetMonoImage(), "GloryEngine.Entities", STRINGIZE(name)); \
if (!out) \
{ \
    Debug::LogError(STRINGIZE(MonoEntitySceneManager::Initialize > Could not load GloryEngine.Entities.##name class!)); \
    return; \
} \
mono_class_init(out);

#define GET_CTOR(name, cls, out, argc) \
iter = NULL; \
method = nullptr; \
while ((method = mono_class_get_methods(cls, &iter))) \
{ \
    const char* methodName = mono_method_get_name(method); \
    if (strcmp(methodName, ".ctor") == 0) \
    { \
        MonoMethodSignature* sig = mono_method_signature(method); \
        if (mono_signature_get_param_count(sig) == argc) \
        { \
            out = method; \
            break; \
        } \
    } \
} \
if (!out) \
{ \
    Debug::LogError(STRINGIZE(MonoEntitySceneManager::Initialize: Could not load GloryEngine.Entities.##name##.::ctor method!)); \
    return; \
}

namespace Glory
{
	/* Cache */
	std::map<GScene*, MonoObject*> MonoSceneManager::m_SceneObjectCache;
	std::map<GScene*, MonoSceneObjectManager*> MonoSceneManager::m_SceneObjectManagers;

	MonoClass* MonoSceneManager::m_pEntitySceneObjectClass = nullptr;
	MonoClass* MonoSceneManager::m_pEntitySceneClass = nullptr;
	MonoMethod* MonoSceneManager::m_pEntitySceneObjectConstructor = nullptr;
	MonoMethod* MonoSceneManager::m_pEntitySceneConstructor = nullptr;

	MonoObject* MonoSceneManager::GetSceneObject(GScene* pScene)
	{
		if (!pScene)
		{
			Debug::LogError("MonoSceneManager::GetSceneObject: Attempting to make scene object for nullptr scene!");
			return nullptr;
		}

		if (m_SceneObjectCache.find(pScene) == m_SceneObjectCache.end())
		{
			MonoObject* pMonoSceneObject = GetSceneObject_Internal(pScene);
			m_SceneObjectCache.emplace(pScene, pMonoSceneObject);
		}

		return m_SceneObjectCache.at(pScene);
	}

	MonoSceneObjectManager* MonoSceneManager::GetSceneObjectManager(GScene* pScene)
	{
		if (!pScene)
		{
			Debug::LogError("MonoSceneManager::GetSceneObjectManager: Attempting to make scene object for nullptr scene!");
			return nullptr;
		}

		if (m_SceneObjectManagers.find(pScene) == m_SceneObjectManagers.end())
		{
			MonoSceneObjectManager* pMonoSceneObject = new MonoSceneObjectManager(pScene);
			m_SceneObjectManagers.emplace(pScene, pMonoSceneObject);
		}

		return m_SceneObjectManagers.at(pScene);
	}

	void MonoSceneManager::DestroyScene(GScene* pScene)
	{

	}

	void MonoSceneManager::Initialize(Assembly* pAssembly)
	{
		GET_CLASS(EntityScene, m_pEntitySceneClass);
		GET_CLASS(EntitySceneObject, m_pEntitySceneObjectClass);

		void* iter = NULL;
		MonoMethod* method = nullptr;

		GET_CTOR(EntityScene, m_pEntitySceneClass, m_pEntitySceneConstructor, 1);
		GET_CTOR(EntitySceneObject, m_pEntitySceneObjectClass, m_pEntitySceneObjectConstructor, 2);
	}

	void MonoSceneManager::Cleanup()
	{
		for (auto itor = m_SceneObjectManagers.begin(); itor != m_SceneObjectManagers.end(); ++itor)
		{
			delete itor->second;
		}
		m_SceneObjectManagers.clear();

		/* @todo: Destroy all scene objects */
		m_SceneObjectCache.clear();
	}

	MonoObject* MonoSceneManager::GetSceneObject_Internal(GScene* pScene)
	{
		MonoObject* pMonoObject = mono_object_new(mono_domain_get(), m_pEntitySceneClass);
		if (pMonoObject == nullptr)
		{
			Debug::LogError("MonoEntityObjectManager::GetSceneObject_Impl > Failed to create MonoObject from class");
			return nullptr;
		}

		uint64_t sceneID = uint64_t(pScene->GetUUID());
		void* args[1] = {
			&sceneID
		};

		MonoObject* pExcept;
		mono_runtime_invoke(m_pEntitySceneConstructor, pMonoObject, args, &pExcept);
		/* TODO: Handle exception */

		return pMonoObject;
	}
}

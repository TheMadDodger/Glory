#include "MonoEntitySceneManager.h"

#include <MacroHelpers.h>
#include <MonoManager.h>

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
    MonoClass* MonoEntitySceneManager::m_pEntitySceneObjectClass = nullptr;
    MonoClass* MonoEntitySceneManager::m_pEntitySceneClass = nullptr;
    MonoMethod* MonoEntitySceneManager::m_pEntitySceneObjectConstructor = nullptr;
    MonoMethod* MonoEntitySceneManager::m_pEntitySceneConstructor = nullptr;

    MonoObject* MonoEntitySceneManager::GetSceneObject(GScene* pScene)
    {
        MonoObject* pMonoObject = mono_object_new(MonoManager::GetDomain(), m_pEntitySceneClass);
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

    MonoSceneObjectManager* MonoEntitySceneManager::GetSceneObjectManager(GScene* pScene)
    {
        return new MonoEntityObjectManager(pScene);
    }

    void MonoEntitySceneManager::Initialize(AssemblyBinding* pAssembly)
    {
        GET_CLASS(EntityScene, m_pEntitySceneClass);
        GET_CLASS(EntitySceneObject, m_pEntitySceneObjectClass);

        void* iter = NULL;
        MonoMethod* method = nullptr;

        GET_CTOR(EntityScene, m_pEntitySceneClass, m_pEntitySceneConstructor, 1);
        GET_CTOR(EntitySceneObject, m_pEntitySceneObjectClass, m_pEntitySceneObjectConstructor, 2);
    }

    void MonoEntitySceneManager::Cleanup()
    {
        /* TODO: Cleanup */
    }
}

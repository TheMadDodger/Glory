#include "MonoEntityObjectManager.h"
#include "MonoEntitySceneManager.h"

#include <MonoManager.h>

namespace Glory
{
    MonoEntityObjectManager::MonoEntityObjectManager(GScene* pScene) : MonoSceneObjectManager(pScene)
    {

    }

    MonoEntityObjectManager::~MonoEntityObjectManager()
    {
    }

    MonoObject* MonoEntityObjectManager::GetSceneObject_Impl(SceneObject* pObject)
    {
        MonoObject* pMonoObject = mono_object_new(MonoManager::GetDomain(), MonoEntitySceneManager::EntitySceneObjectClass());
        if (pMonoObject == nullptr)
        {
            Debug::LogError("MonoEntityObjectManager::GetSceneObject_Impl > Failed to create MonoObject from class");
            return nullptr;
        }

        uint64_t objectID = uint64_t(pObject->GetUUID());
        uint64_t sceneID = uint64_t(m_pScene->GetUUID());
        void* args[2] = {
            &objectID,
            &sceneID
        };

        MonoObject* pExcept;
        mono_runtime_invoke(MonoEntitySceneManager::EntitySceneObjectConstructor(), pMonoObject, args, &pExcept);
        /* TODO: Handle exception */

        return pMonoObject;
    }

    void MonoEntityObjectManager::DestroySceneObject_Impl(MonoObject* pMonoObject)
    {
    }
}

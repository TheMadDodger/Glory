#include "MonoScriptObjectManager.h"
#include "MonoManager.h"

namespace Glory
{
    std::map<Object*, ObjectInstanceData> MonoScriptObjectManager::m_Objects;
    std::map<MonoClass*, MonoObject*> MonoScriptObjectManager::m_pDummyScriptObjects;
    std::map<MonoObject*, Object*> MonoScriptObjectManager::m_pMonoToObject;

    MonoObject* MonoScriptObjectManager::GetScriptDummyObject(MonoClass* pClass)
    {
        if (m_pDummyScriptObjects.find(pClass) == m_pDummyScriptObjects.end())
            return CreateScriptDummyObject(pClass);
        return m_pDummyScriptObjects[pClass];
    }

    MonoObject* MonoScriptObjectManager::GetScriptObject(MonoClass* pClass, Object* pObject)
    {
        if (m_Objects.find(pObject) == m_Objects.end() || m_Objects[pObject].m_pObjects.find(pClass) == m_Objects[pObject].m_pObjects.end())
            return CreateScriptObject(pClass, pObject);
        return m_Objects[pObject].m_pObjects[pClass];
    }

    Object* MonoScriptObjectManager::GetScriptObject(MonoObject* pMonoObject)
    {
        if (m_pMonoToObject.find(pMonoObject) == m_pMonoToObject.end()) return nullptr;
        return m_pMonoToObject[pMonoObject];
    }

    void MonoScriptObjectManager::DestroyScriptObject(MonoClass* pClass, Object* pObject)
    {
        if (m_Objects.find(pObject) == m_Objects.end() || m_Objects[pObject].m_pObjects.find(pClass) == m_Objects[pObject].m_pObjects.end()) return;
        MonoObject* pMonoObject = m_Objects[pObject].m_pObjects[pClass];

        /* TODO: Cleanup the GC Handle */

        m_Objects[pObject].m_pObjects.erase(pClass);
        m_pMonoToObject.erase(pMonoObject);
    }

    void MonoScriptObjectManager::Cleanup()
    {
        m_Objects.clear();
    }

    MonoObject* MonoScriptObjectManager::CreateScriptObject(MonoClass* pClass, Object* pObject)
    {
        MonoDomain* pDomain = mono_domain_get();
        MonoObject* pMonoObject = mono_object_new(pDomain, pClass);
        if (pMonoObject == nullptr)
        {
            Debug::LogError("MonoScriptObjectManager::CreateObject > Failed to create MonoObject from class");
            return nullptr;
        }
        mono_runtime_object_init(pMonoObject);
        m_Objects[pObject].m_pObjects[pClass] = pMonoObject;
        m_pMonoToObject[pMonoObject] = pObject;
        return pMonoObject;
    }

    MonoObject* MonoScriptObjectManager::CreateScriptDummyObject(MonoClass* pClass)
    {
        MonoDomain* pDomain = mono_domain_get();
        MonoObject* pMonoObject = mono_object_new(pDomain, pClass);
        if (pMonoObject == nullptr)
        {
            Debug::LogError("MonoScriptObjectManager::CreateDummyObject > Failed to create MonoObject from class");
            return nullptr;
        }
        mono_runtime_object_init(pMonoObject);
        m_pDummyScriptObjects.emplace(pClass, pMonoObject);
        return pMonoObject;
    }

    MonoScriptObjectManager::MonoScriptObjectManager()
    {
    }

    MonoScriptObjectManager::~MonoScriptObjectManager()
    {
    }
}

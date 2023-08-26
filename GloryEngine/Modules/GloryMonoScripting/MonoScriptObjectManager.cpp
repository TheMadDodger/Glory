#include "MonoScriptObjectManager.h"
#include "MonoManager.h"

namespace Glory
{
    MonoObject* MonoScriptObjectManager::GetScriptDummyObject(MonoClass* pClass)
    {
        auto itor = m_pDummyScriptObjects.find(pClass);
        if (itor == m_pDummyScriptObjects.end())
            return CreateScriptDummyObject(pClass);

        return itor->second;
    }

    MonoObject* MonoScriptObjectManager::GetScriptObject(MonoClass* pClass, Object* pObject)
    {
        auto itor = m_Objects.find(pObject);
        if (itor == m_Objects.end())
        {
            auto newItor = m_Objects.emplace(pObject, ObjectInstanceData{}).first;
            return CreateScriptObject(newItor->second, pClass, pObject);
        }

        auto classItor = itor->second.m_pObjects.find(pClass);
        if (classItor == itor->second.m_pObjects.end())
            return CreateScriptObject(itor->second, pClass, pObject);

        return classItor->second.first;
    }

    Object* MonoScriptObjectManager::GetScriptObject(MonoObject* pMonoObject)
    {
        auto itor = m_pMonoToObject.find(pMonoObject);
        if (itor == m_pMonoToObject.end()) return nullptr;
        return itor->second;
    }

    void MonoScriptObjectManager::DestroyScriptObject(MonoClass* pClass, Object* pObject)
    {
        auto itor = m_Objects.find(pObject);
        if (itor == m_Objects.end()) return;
        auto objectItor = itor->second.m_pObjects.find(pClass);

        if (objectItor == itor->second.m_pObjects.end()) return;
        MonoObject* pMonoObject = objectItor->second.first;

        mono_gchandle_free(objectItor->second.second);

        itor->second.m_pObjects.erase(objectItor);
        m_pMonoToObject.erase(pMonoObject);
    }

    void MonoScriptObjectManager::DestroyAllObjects()
    {
        for (auto& itor : m_Objects)
        {
            for (auto& objectItor : itor.second.m_pObjects)
            {
                mono_gchandle_free(objectItor.second.second);
            }
        }
        m_Objects.clear();
    }

    void MonoScriptObjectManager::Cleanup()
    {
        DestroyAllObjects();
    }

    MonoObject* MonoScriptObjectManager::CreateScriptObject(ObjectInstanceData& instanceData, MonoClass* pClass, Object* pObject)
    {
        MonoDomain* pDomain = mono_domain_get();
        MonoObject* pMonoObject = mono_object_new(pDomain, pClass);
        if (pMonoObject == nullptr)
        {
            Debug::LogError("MonoScriptObjectManager::CreateObject > Failed to create MonoObject from class");
            return nullptr;
        }
        mono_runtime_object_init(pMonoObject);
        const uint32_t gcHandle = mono_gchandle_new(pMonoObject, false);

        instanceData.m_pObjects.emplace(pClass, std::pair<MonoObject*, uint32_t>{ pMonoObject, gcHandle });
        m_pMonoToObject.emplace(pMonoObject, pObject);
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

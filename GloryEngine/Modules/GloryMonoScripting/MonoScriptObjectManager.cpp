#include "MonoScriptObjectManager.h"
#include "MonoManager.h"

#include <Engine.h>
#include <Debug.h>

namespace Glory
{
    MonoObject* MonoScriptObjectManager::GetMonoScriptDummyObject(MonoClass* pClass)
    {
        auto itor = m_pDummyScriptObjects.find(pClass);
        if (itor == m_pDummyScriptObjects.end())
            return CreateMonoScriptDummyObject(pClass);

        return itor->second;
    }

    MonoObject* MonoScriptObjectManager::GetMonoScriptObject(MonoClass* pClass, UUID uuid, UUID sceneID)
    {
        auto itor = m_Objects.find(uuid);
        if (itor == m_Objects.end())
        {
            auto newItor = m_Objects.emplace(uuid, ObjectInstanceData{}).first;
            MonoObject* pObject = CreateMonoScriptObject(newItor->second, pClass, uuid);
            if (pObject) m_pMonoObjectScenes.emplace(pObject, sceneID);
            return pObject;
        }

        auto classItor = itor->second.m_pObjects.find(pClass);
        if (classItor == itor->second.m_pObjects.end())
        {
            MonoObject* pObject = CreateMonoScriptObject(itor->second, pClass, uuid);
            if (pObject) m_pMonoObjectScenes.emplace(pObject, sceneID);
            return pObject;
        }

        return classItor->second.first;
    }

    UUID MonoScriptObjectManager::GetIDForMonoScriptObject(MonoObject* pMonoObject) const
    {
        auto itor = m_pMonoToObject.find(pMonoObject);
        if (itor == m_pMonoToObject.end()) return 0;
        return itor->second;
    }

    UUID MonoScriptObjectManager::GetSceneIDForMonoScriptObject(MonoObject* pMonoObject) const
    {
        auto itor = m_pMonoObjectScenes.find(pMonoObject);
        if (itor == m_pMonoObjectScenes.end()) return 0;
        return itor->second;
    }

    void MonoScriptObjectManager::DestroyScriptObject(MonoClass* pClass, UUID uuid)
    {
        auto itor = m_Objects.find(uuid);
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

    MonoObject* MonoScriptObjectManager::CreateMonoScriptObject(ObjectInstanceData& instanceData, MonoClass* pClass, UUID uuid)
    {
        MonoDomain* pDomain = mono_domain_get();
        MonoObject* pMonoObject = mono_object_new(pDomain, pClass);
        if (pMonoObject == nullptr)
        {
            MonoManager::Instance()->Module()->GetEngine()->GetDebug().LogError("MonoScriptObjectManager::CreateObject > Failed to create MonoObject from class");
            return nullptr;
        }
        mono_runtime_object_init(pMonoObject);
        const uint32_t gcHandle = mono_gchandle_new(pMonoObject, false);

        instanceData.m_pObjects.emplace(pClass, std::pair<MonoObject*, uint32_t>{ pMonoObject, gcHandle });
        m_pMonoToObject.emplace(pMonoObject, uuid);
        return pMonoObject;
    }

    MonoObject* MonoScriptObjectManager::CreateMonoScriptDummyObject(MonoClass* pClass)
    {
        MonoDomain* pDomain = mono_domain_get();
        MonoObject* pMonoObject = mono_object_new(pDomain, pClass);
        if (pMonoObject == nullptr)
        {
            MonoManager::Instance()->Module()->GetEngine()->GetDebug().LogError("MonoScriptObjectManager::CreateDummyObject > Failed to create MonoObject from class");
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

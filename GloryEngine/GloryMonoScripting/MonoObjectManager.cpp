#include "MonoObjectManager.h"
#include "MonoLibManager.h"
#include "MonoManager.h"

namespace Glory
{
    std::map<Object*, std::map<MonoClass*, MonoObject*>> MonoObjectManager::m_Objects;

    MonoObject* MonoObjectManager::GetObject(MonoClass* pClass, Object* pObject)
    {
        if (m_Objects.find(pObject) == m_Objects.end() || m_Objects[pObject].find(pClass) == m_Objects[pObject].end())
            return CreateObject(pClass, pObject);
        return m_Objects[pObject][pClass];
    }

    void MonoObjectManager::DestroyObject(MonoClass* pClass, Object* pObject)
    {
        if (m_Objects.find(pObject) == m_Objects.end() || m_Objects[pObject].find(pClass) == m_Objects[pObject].end()) return;
        MonoObject* pMonoObject = m_Objects[pObject][pClass];
    }

    void MonoObjectManager::Cleanup()
    {
    }

    MonoObject* MonoObjectManager::CreateObject(MonoClass* pClass, Object* pObject)
    {
        MonoDomain* pDomain = MonoManager::GetDomain();
        MonoObject* pMonoObject = mono_object_new(pDomain, pClass);
        m_Objects[pObject][pClass] = pMonoObject;
        return pMonoObject;
    }

    MonoObjectManager::MonoObjectManager()
    {
    }

    MonoObjectManager::~MonoObjectManager()
    {
    }
}

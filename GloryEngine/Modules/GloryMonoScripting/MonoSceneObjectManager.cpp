#include "MonoSceneObjectManager.h"

namespace Glory
{
	MonoSceneObjectManager::~MonoSceneObjectManager()
	{
		/* TODO: Release GC Handles? */
		m_ObjectsCache.clear();
	}

	MonoObject* MonoSceneObjectManager::GetSceneObject(SceneObject* pObject)
	{
		if (m_ObjectsCache.find(pObject) == m_ObjectsCache.end())
		{
			MonoObject* pMonoObject = GetSceneObject_Impl(pObject);
			if (!pMonoObject) return nullptr;
			m_ObjectsCache.emplace(pObject, pMonoObject);
		}

		return m_ObjectsCache.at(pObject);
	}

	void MonoSceneObjectManager::DestroySceneObject(SceneObject* pObject)
	{
		if (m_ObjectsCache.find(pObject) == m_ObjectsCache.end()) return;
		DestroySceneObject_Impl(m_ObjectsCache.at(pObject));
		m_ObjectsCache.erase(pObject);
	}
}

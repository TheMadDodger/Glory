#include "MonoSceneObjectManager.h"
#include "MonoSceneManager.h"

#include <Debug.h>

namespace Glory
{
	MonoSceneObjectManager::~MonoSceneObjectManager()
	{
		/* @todo: Release GC Handles? */
		m_ObjectsCache.clear();
	}

	MonoObject* MonoSceneObjectManager::GetSceneObject(SceneObject* pObject)
	{
		if (m_ObjectsCache.find(pObject) == m_ObjectsCache.end())
		{
			MonoObject* pMonoObject = GetSceneObject_Internal(pObject);
			if (!pMonoObject) return nullptr;
			m_ObjectsCache.emplace(pObject, pMonoObject);
		}

		return m_ObjectsCache.at(pObject);
	}

	void MonoSceneObjectManager::DestroySceneObject(SceneObject* pObject)
	{
		if (m_ObjectsCache.find(pObject) == m_ObjectsCache.end()) return;
		DestroySceneObject_Internal(m_ObjectsCache.at(pObject));
		m_ObjectsCache.erase(pObject);
	}

	MonoObject* MonoSceneObjectManager::GetSceneObject_Internal(SceneObject* pObject)
	{
		MonoObject* pMonoObject = mono_object_new(mono_domain_get(), MonoSceneManager::EntitySceneObjectClass());
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
		mono_runtime_invoke(MonoSceneManager::EntitySceneObjectConstructor(), pMonoObject, args, &pExcept);
		/* @todo: Handle exception */

		return pMonoObject;
	}

	void MonoSceneObjectManager::DestroySceneObject_Internal(MonoObject* pMonoObject)
	{
	}
}

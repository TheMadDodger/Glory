#include "MonoSceneObjectManager.h"
#include "MonoSceneManager.h"

#include <Debug.h>
#include <SceneManager.h>

namespace Glory
{
	MonoSceneObjectManager::~MonoSceneObjectManager()
	{
		/* @todo: Release GC Handles? */
		m_ObjectsCache.clear();
	}

	MonoObject* MonoSceneObjectManager::GetMonoSceneObject(UUID objectID)
	{
		if (m_ObjectsCache.find(objectID) == m_ObjectsCache.end())
		{
			MonoObject* pMonoObject = GetMonoSceneObject_Internal(objectID);
			if (!pMonoObject) return nullptr;
			m_ObjectsCache.emplace(objectID, pMonoObject);
		}

		return m_ObjectsCache.at(objectID);
	}

	void MonoSceneObjectManager::DestroyMonoSceneObject(UUID objectID)
	{
		if (m_ObjectsCache.find(objectID) == m_ObjectsCache.end()) return;
		DestroyMonoSceneObject_Internal(m_ObjectsCache.at(objectID));
		m_ObjectsCache.erase(objectID);
	}

	MonoObject* MonoSceneObjectManager::GetMonoSceneObject_Internal(UUID objectID)
	{
		MonoObject* pMonoObject = mono_object_new(mono_domain_get(), MonoSceneManager::EntitySceneObjectClass());
		if (pMonoObject == nullptr)
		{
			m_pScene->Manager()->GetEngine()->GetDebug().LogError("MonoEntityObjectManager::GetSceneObject_Impl > Failed to create MonoObject from class");
			return nullptr;
		}

		uint64_t object = uint64_t(objectID);
		uint64_t sceneID = uint64_t(m_pScene->GetUUID());
		void* args[2] = {
			&object,
			&sceneID
		};

		MonoObject* pExcept;
		mono_runtime_invoke(MonoSceneManager::EntitySceneObjectConstructor(), pMonoObject, args, &pExcept);
		/* @todo: Handle exception */

		return pMonoObject;
	}

	void MonoSceneObjectManager::DestroyMonoSceneObject_Internal(MonoObject* pMonoObject)
	{
	}
}

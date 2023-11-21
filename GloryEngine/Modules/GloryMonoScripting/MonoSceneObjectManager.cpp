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

	MonoObject* MonoSceneObjectManager::GetSceneObject(UUID objectID)
	{
		if (m_ObjectsCache.find(objectID) == m_ObjectsCache.end())
		{
			MonoObject* pMonoObject = GetSceneObject_Internal(objectID);
			if (!pMonoObject) return nullptr;
			m_ObjectsCache.emplace(objectID, pMonoObject);
		}

		return m_ObjectsCache.at(objectID);
	}

	void MonoSceneObjectManager::DestroySceneObject(UUID objectID)
	{
		if (m_ObjectsCache.find(objectID) == m_ObjectsCache.end()) return;
		DestroySceneObject_Internal(m_ObjectsCache.at(objectID));
		m_ObjectsCache.erase(objectID);
	}

	MonoObject* MonoSceneObjectManager::GetSceneObject_Internal(UUID objectID)
	{
		MonoObject* pMonoObject = mono_object_new(mono_domain_get(), MonoSceneManager::EntitySceneObjectClass());
		if (pMonoObject == nullptr)
		{
			Debug::LogError("MonoEntityObjectManager::GetSceneObject_Impl > Failed to create MonoObject from class");
			return nullptr;
		}

		uint64_t objectID = uint64_t(objectID);
		uint64_t sceneID = uint64_t(objectID);
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

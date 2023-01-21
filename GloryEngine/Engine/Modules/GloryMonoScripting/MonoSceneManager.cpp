#include "MonoSceneManager.h"

#define BINDER_DEF(name, ret, ...) \
std::function<ret(__VA_ARGS__)> MonoSceneManager::m_##name##Impl = NULL; \
void MonoSceneManager::Bind##name(std::function<ret(__VA_ARGS__)> f) \
{ \
	m_##name##Impl = f; \
}

namespace Glory
{
	/* Implementation binds */
	BINDER_DEF(GetSceneObject, MonoObject*, GScene*);
	BINDER_DEF(GetSceneObjectManager, MonoSceneObjectManager*, GScene*);

	/* Cache */
	std::map<GScene*, MonoObject*> MonoSceneManager::m_SceneObjectCache;
	std::map<GScene*, MonoSceneObjectManager*> MonoSceneManager::m_SceneObjectManagers;

	bool MonoSceneManager::m_Bound = false;

	MonoObject* MonoSceneManager::GetSceneObject(GScene* pScene)
	{
		if (!pScene)
		{
			Debug::LogError("MonoSceneManager::GetSceneObject: Attempting to make scene object for nullptr scene!");
			return nullptr;
		}

		if (m_SceneObjectCache.find(pScene) == m_SceneObjectCache.end())
		{
			if (m_GetSceneObjectImpl == NULL)
			{
				Debug::LogError("MonoSceneManager::GetSceneObject: No extension loaded that implements scene management!");
				return nullptr;
			}

			MonoObject* pMonoSceneObject = m_GetSceneObjectImpl(pScene);
			m_SceneObjectCache.emplace(pScene, pMonoSceneObject);
		}

		return m_SceneObjectCache.at(pScene);
	}

	MonoSceneObjectManager* MonoSceneManager::GetSceneObjectManager(GScene* pScene)
	{
		if (!pScene)
		{
			Debug::LogError("MonoSceneManager::GetSceneObjectManager: Attempting to make scene object for nullptr scene!");
			return nullptr;
		}

		if (m_SceneObjectManagers.find(pScene) == m_SceneObjectManagers.end())
		{
			if (m_GetSceneObjectManagerImpl == NULL)
			{
				Debug::LogError("MonoSceneManager::GetSceneObjectManager: No extension loaded that implements scene management!");
				return nullptr;
			}

			MonoSceneObjectManager* pMonoSceneObject = m_GetSceneObjectManagerImpl(pScene);
			m_SceneObjectManagers.emplace(pScene, pMonoSceneObject);
		}

		return m_SceneObjectManagers.at(pScene);
	}

	void MonoSceneManager::DestroyScene(GScene* pScene)
	{

	}

	void MonoSceneManager::UnbindImplementation()
	{
		m_GetSceneObjectImpl = NULL;
		m_GetSceneObjectManagerImpl = NULL;
		m_Bound = false;
	}

	void MonoSceneManager::Cleanup()
	{
		for (auto itor = m_SceneObjectManagers.begin(); itor != m_SceneObjectManagers.end(); ++itor)
		{
			delete itor->second;
		}
		m_SceneObjectManagers.clear();

		/* TODO: Destroy all scene objects */
		m_SceneObjectCache.clear();

		UnbindImplementation();
	}

	void MonoSceneManager::CheckBound()
	{
		if (m_Bound)
		{
			Debug::LogWarning("MonoSceneManager::BindImplemetation: Other extension has already bound a scene implementation");
			return;
		}
		m_Bound = true;
	}
}

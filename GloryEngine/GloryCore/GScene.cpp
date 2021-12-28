#include "GScene.h"
#include <algorithm>

namespace Glory
{
	GScene::GScene() : m_SceneName("New Scene")
	{
		APPEND_TYPE(GScene);
	}

	GScene::GScene(const std::string& sceneName) : m_SceneName(sceneName)
	{
		APPEND_TYPE(GScene);
	}

	GScene::GScene(const std::string& sceneName, UUID uuid) : Resource(uuid), m_SceneName(sceneName)
	{
		APPEND_TYPE(GScene);
	}

	GScene::~GScene()
	{
		std::for_each(m_pSceneObjects.begin(), m_pSceneObjects.end(), [](SceneObject* pObject) { delete pObject; });
	}

	SceneObject* GScene::CreateEmptyObject()
	{
		SceneObject* pObject = CreateObject("Empty Object");
		pObject->Initialize();
		m_pSceneObjects.push_back(pObject);
		OnObjectAdded(pObject);
		return pObject;
	}

	SceneObject* GScene::CreateEmptyObject(const std::string& name, UUID uuid)
	{
		SceneObject* pObject = CreateObject(name, uuid);
		pObject->Initialize();
		m_pSceneObjects.push_back(pObject);
		OnObjectAdded(pObject);
		return pObject;
	}

	size_t GScene::SceneObjectsCount()
	{
		return m_pSceneObjects.size();
	}

	SceneObject* GScene::GetSceneObject(size_t index)
	{
		if (index >= m_pSceneObjects.size()) return nullptr;
		return m_pSceneObjects[index];
	}

	const std::string& GScene::Name()
	{
		return m_SceneName;
	}
}
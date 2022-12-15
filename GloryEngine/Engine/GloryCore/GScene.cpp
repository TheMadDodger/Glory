#include "GScene.h"
#include "Debug.h"
#include <algorithm>

namespace Glory
{
	GScene::GScene(const std::string& sceneName) : Resource(sceneName)
	{
		APPEND_TYPE(GScene);
	}

	GScene::GScene(const std::string& sceneName, UUID uuid) : Resource(uuid, sceneName)
	{
		APPEND_TYPE(GScene);
	}

	GScene::~GScene()
	{
		std::for_each(m_pSceneObjects.begin(), m_pSceneObjects.end(), [](SceneObject* pObject) { delete pObject; });
		m_pSceneObjects.clear();
	}

	SceneObject* GScene::CreateEmptyObject()
	{
		SceneObject* pObject = CreateObject("Empty Object");
		pObject->m_pScene = this;
		m_pSceneObjects.push_back(pObject);
		pObject->Initialize();
		OnObjectAdded(pObject);
		return pObject;
	}

	SceneObject* GScene::CreateEmptyObject(const std::string& name, UUID uuid, UUID uuid2)
	{
		SceneObject* pObject = CreateObject(name, uuid, uuid2);
		pObject->m_pScene = this;
		m_pSceneObjects.push_back(pObject);
		pObject->Initialize();
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

	void GScene::DeleteObject(SceneObject* pObject)
	{
		OnDeleteObject(pObject);
		auto it = std::find(m_pSceneObjects.begin(), m_pSceneObjects.end(), pObject);
		if (it == m_pSceneObjects.end())
		{
			Debug::LogError("Can't delete object from scene that does not own it!");
			return;
		}
		m_pSceneObjects.erase(it);
		pObject->DestroyOwnChildren();
		pObject->SetParent(nullptr);
		delete pObject;
	}

	SceneObject* GScene::FindSceneObject(UUID uuid) const
	{
		auto it = std::find_if(m_pSceneObjects.begin(), m_pSceneObjects.end(), [&](SceneObject* pObject) { return pObject->GetUUID() == uuid; });
		if (it == m_pSceneObjects.end()) return nullptr;
		return *it;
	}

	void GScene::DelayedSetParent(SceneObject* pObjectToParent, UUID parentID)
	{
		if (pObjectToParent == nullptr || parentID == NULL) return;
		m_DelayedParents.emplace_back(DelayedParentData(pObjectToParent, parentID));
	}

	void GScene::HandleDelayedParents()
	{
		std::for_each(m_DelayedParents.begin(), m_DelayedParents.end(), [&](const DelayedParentData& data) { OnDelayedSetParent(data); });
		m_DelayedParents.clear();
	}

	void GScene::OnDelayedSetParent(const DelayedParentData& data)
	{
		SceneObject* pParent = FindSceneObject(data.ParentID);
		if (pParent == nullptr)
		{
			Debug::LogError("Could not set delayed parent for object " + data.ObjectToParent->Name() + " because the parent does not exist!");
			return;
		}
		data.ObjectToParent->SetParent(pParent);
	}

	void GScene::SetUUID(UUID uuid)
	{
		m_ID = uuid;
	}
}
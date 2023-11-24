//#include "GScene.h"
//#include "Debug.h"
//#include "Components.h"
//#include "PrefabData.h"
//#include "DistributedRandom.h"
//#include "SceneObject.h"
//#include "PropertySerializer.h"
//
//#include <algorithm>
//#include <NodeRef.h>
//
//namespace Glory
//{
//	void GScene::DeleteObject(SceneObject* pObject)
//	{
//		OnDeleteObject(pObject);
//		auto it = std::find(m_pSceneObjects.begin(), m_pSceneObjects.end(), pObject);
//		if (it == m_pSceneObjects.end())
//		{
//			Debug::LogError("Can't delete object from scene that does not own it!");
//			return;
//		}
//		m_pSceneObjects.erase(it);
//		pObject->DestroyOwnChildren();
//		pObject->SetParent(nullptr);
//		delete pObject;
//	}
//
//	void GScene::DelayedSetParent(SceneObject* pObjectToParent, UUID parentID)
//	{
//		if (pObjectToParent == nullptr || parentID == NULL) return;
//		m_DelayedParents.emplace_back(DelayedParentData(pObjectToParent, parentID));
//	}
//
//	void GScene::HandleDelayedParents()
//	{
//		std::for_each(m_DelayedParents.begin(), m_DelayedParents.end(), [&](const DelayedParentData& data) { OnDelayedSetParent(data); });
//		m_DelayedParents.clear();
//	}
//
//	void GScene::OnDelayedSetParent(const DelayedParentData& data)
//	{
//		SceneObject* pParent = FindSceneObject(data.ParentID);
//		if (pParent == nullptr)
//		{
//			Debug::LogError("Could not set delayed parent for object " + data.ObjectToParent->Name() + " because the parent does not exist!");
//			return;
//		}
//		data.ObjectToParent->SetParent(pParent);
//	}
//}
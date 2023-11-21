//#include "SceneObject.h"
//#include "GScene.h"
//#include "Debug.h"
//#include "Components.h"
//
//namespace Glory
//{
//	void SceneObject::SetSiblingIndex(size_t index)
//	{
//		SceneObject* pParent = GetParent();
//
//		std::vector<SceneObject*>* targetVector = nullptr;
//		if (pParent == nullptr)
//			targetVector = &m_pScene->m_pSceneObjects;
//		else
//			targetVector = &pParent->m_pChildren;
//
//		auto it = std::find(targetVector->begin(), targetVector->end(), this);
//		if (it == targetVector->end()) return;
//		targetVector->erase(it);
//		targetVector->insert(targetVector->begin() + index, this);
//	}
//
//	void SceneObject::SetBeforeObject(SceneObject* pObject)
//	{
//		SceneObject* pParent = GetParent();
//
//		std::vector<SceneObject*>* targetVector = &m_pScene->m_pSceneObjects;
//		if (pParent != nullptr)
//			targetVector = &pParent->m_pChildren;
//
//		auto it = std::find(targetVector->begin(), targetVector->end(), this);
//		if (it == targetVector->end()) return;
//		targetVector->erase(it);
//
//		auto targetIterator = std::find(targetVector->begin(), targetVector->end(), pObject);
//		if (targetIterator == targetVector->end())
//		{
//			targetVector->push_back(this);
//			return;
//		}
//		targetVector->insert(targetIterator, this);
//	}
//
//	void SceneObject::SetAfterObject(SceneObject* pObject)
//	{
//		SceneObject* pParent = GetParent();
//
//		std::vector<SceneObject*>* targetVector = &m_pScene->m_pSceneObjects;
//		if (pParent != nullptr)
//			targetVector = &pParent->m_pChildren;
//
//		auto it = std::find(targetVector->begin(), targetVector->end(), this);
//		if (it == targetVector->end()) return;
//		targetVector->erase(it);
//
//		auto targetIterator = std::find(targetVector->begin(), targetVector->end(), pObject);
//
//		if (targetIterator == targetVector->end() || targetIterator + 1 == targetVector->end())
//		{
//			targetVector->push_back(this);
//			return;
//		}
//		targetVector->insert(targetIterator + 1, this);
//	}
//
//	size_t SceneObject::GetSiblingIndex()
//	{
//		SceneObject* pParent = GetParent();
//		std::vector<SceneObject*>* targetVector = nullptr;
//		if (pParent == nullptr)
//			targetVector = &m_pScene->m_pSceneObjects;
//		else
//			targetVector = &pParent->m_pChildren;
//
//		auto it = std::find(targetVector->begin(), targetVector->end(), this);
//		return it - targetVector->begin();
//	}
// 
//	void SceneObject::OnSetParent(SceneObject* pParent)
//	{
//		if (!m_Entity.HasComponent<Transform>())
//		{
//			Debug::LogError("Cant add parent to an entity with no Transform component!");
//			return;
//		}
//
//		Transform& transform = m_Entity.GetComponent<Transform>();
//
//		if (pParent == nullptr)
//		{
//			transform.Parent = Entity();
//			m_pParent = nullptr;
//			SetHierarchyActive();
//			return;
//		}
//
//		Entity parentHandle = pParent->GetEntityHandle();
//		if (!parentHandle.HasComponent<Transform>())
//		{
//			Debug::LogError("Cant parent entity to an entity with no Transform component!");
//			return;
//		}
//
//		m_pParent = pParent;
//		transform.Parent = parentHandle;
//		SetHierarchyActive();
//	}
//
//	void SceneObject::DestroyOwnChildren()
//	{
//		const size_t size = m_pChildren.size();
//		if (size == 0) return;
//		SceneObject** pChildren = new SceneObject*[size];
//		memcpy(pChildren, m_pChildren.data(), m_pChildren.size() * sizeof(SceneObject*));
//		for (size_t i = 0; i < size; i++)
//		{
//			m_pScene->DeleteObject(pChildren[i]);
//		}
//		m_pChildren.clear();
//		delete[] pChildren;
//	}
//
//	void SceneObject::SetParent(SceneObject* pParent)
//	{
//		SceneObject* pCurrentParent = GetParent();
//		if (pCurrentParent != nullptr)
//		{
//			auto it = std::find(pCurrentParent->m_pChildren.begin(), pCurrentParent->m_pChildren.end(), this);
//			if (it != pCurrentParent->m_pChildren.end())
//				pCurrentParent->m_pChildren.erase(it);
//		}
//
//		if (pParent != nullptr) pParent->m_pChildren.push_back(this);
//		OnSetParent(pParent);
//	}
//
//	bool SceneObject::IsActiveSelf() const
//	{
//		return m_Entity.IsActiveSelf();
//	}
//
//	bool SceneObject::IsActiveInHierarchy() const
//	{
//		return m_Entity.IsActive();
//	}
//
//	void SceneObject::SetActive(bool active)
//	{
//		m_Entity.SetActive(active);
//		SetHierarchyActive();
//	}
//
//	void SceneObject::SetHierarchyActive()
//	{
//		SceneObject* pParent = GetParent();
//		const bool active = (!pParent || pParent->IsActiveInHierarchy()) && IsActiveSelf();
//		m_Entity.SetActiveHierarchy(active);
//
//		for (size_t i = 0; i < ChildCount(); i++)
//		{
//			GetChild(i)->SetHierarchyActive();
//		}
//	}
//
//	Entity SceneObject::GetEntityHandle()
//	{
//		return m_Entity;
//	}
//}
//
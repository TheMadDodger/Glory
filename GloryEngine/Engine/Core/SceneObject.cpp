#include "SceneObject.h"
#include "GScene.h"

namespace Glory
{
	SceneObject::SceneObject() : Object("Empty Object"), m_pScene(nullptr)
	{
		APPEND_TYPE(SceneObject);
	}

	SceneObject::SceneObject(const std::string& name) : Object(name), m_pScene(nullptr)
	{
		APPEND_TYPE(SceneObject);
	}

	SceneObject::SceneObject(const std::string& name, UUID uuid) : Object(uuid, name), m_pScene(nullptr)
	{
		APPEND_TYPE(SceneObject);
	}

	SceneObject::~SceneObject()
	{
	}

	size_t SceneObject::ChildCount()
	{
		return m_pChildren.size();
	}

	SceneObject* SceneObject::GetChild(size_t index)
	{
		return m_pChildren[index];
	}

	void SceneObject::SetSiblingIndex(size_t index)
	{
		SceneObject* pParent = GetParent();

		std::vector<SceneObject*>* targetVector = nullptr;
		if (pParent == nullptr)
			targetVector = &m_pScene->m_pSceneObjects;
		else
			targetVector = &pParent->m_pChildren;

		auto it = std::find(targetVector->begin(), targetVector->end(), this);
		if (it == targetVector->end()) return;
		targetVector->erase(it);
		targetVector->insert(targetVector->begin() + index, this);
	}

	void SceneObject::SetBeforeObject(SceneObject* pObject)
	{
		SceneObject* pParent = GetParent();

		std::vector<SceneObject*>* targetVector = &m_pScene->m_pSceneObjects;
		if (pParent != nullptr)
			targetVector = &pParent->m_pChildren;

		auto it = std::find(targetVector->begin(), targetVector->end(), this);
		if (it == targetVector->end()) return;
		targetVector->erase(it);

		auto targetIterator = std::find(targetVector->begin(), targetVector->end(), pObject);
		if (targetIterator == targetVector->end())
		{
			targetVector->push_back(this);
			return;
		}
		targetVector->insert(targetIterator, this);
	}

	void SceneObject::SetAfterObject(SceneObject* pObject)
	{
		SceneObject* pParent = GetParent();

		std::vector<SceneObject*>* targetVector = &m_pScene->m_pSceneObjects;
		if (pParent != nullptr)
			targetVector = &pParent->m_pChildren;

		auto it = std::find(targetVector->begin(), targetVector->end(), this);
		if (it == targetVector->end()) return;
		targetVector->erase(it);

		auto targetIterator = std::find(targetVector->begin(), targetVector->end(), pObject);

		if (targetIterator == targetVector->end() || targetIterator + 1 == targetVector->end())
		{
			targetVector->push_back(this);
			return;
		}
		targetVector->insert(targetIterator + 1, this);
	}

	size_t SceneObject::GetSiblingIndex()
	{
		SceneObject* pParent = GetParent();
		std::vector<SceneObject*>* targetVector = nullptr;
		if (pParent == nullptr)
			targetVector = &m_pScene->m_pSceneObjects;
		else
			targetVector = &pParent->m_pChildren;

		auto it = std::find(targetVector->begin(), targetVector->end(), this);
		return it - targetVector->begin();
	}

	void SceneObject::SetScene(GScene* pScene)
	{
		if (m_pScene)
		{
			auto it = std::find(m_pScene->m_pSceneObjects.begin(), m_pScene->m_pSceneObjects.end(), this);
			if (it != m_pScene->m_pSceneObjects.end()) m_pScene->m_pSceneObjects.erase(it);
		}

		m_pScene = pScene;
		m_pScene->m_pSceneObjects.push_back(this);
	}

	GScene* SceneObject::GetScene() const
	{
		return m_pScene;
	}

	void SceneObject::DestroyOwnChildren()
	{
		const size_t size = m_pChildren.size();
		if (size == 0) return;
		SceneObject** pChildren = new SceneObject*[size];
		memcpy(pChildren, m_pChildren.data(), m_pChildren.size() * sizeof(SceneObject*));
		for (size_t i = 0; i < size; i++)
		{
			m_pScene->DeleteObject(pChildren[i]);
		}
		m_pChildren.clear();
		delete[] pChildren;
	}

	void SceneObject::SetParent(SceneObject* pParent)
	{
		SceneObject* pCurrentParent = GetParent();
		if (pCurrentParent != nullptr)
		{
			auto it = std::find(pCurrentParent->m_pChildren.begin(), pCurrentParent->m_pChildren.end(), this);
			if (it != pCurrentParent->m_pChildren.end())
				pCurrentParent->m_pChildren.erase(it);
		}

		if (pParent != nullptr) pParent->m_pChildren.push_back(this);
		OnSetParent(pParent);
	}
}

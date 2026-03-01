#include "ObjectManager.h"
#include "Object.h"

#include <algorithm>

namespace Glory
{
	void ObjectManager::Add(Object* pObject)
	{
		m_pAllObjects.emplace_back(pObject);
	}

	void ObjectManager::Remove(Object* pObject)
	{
		auto it = std::find(m_pAllObjects.begin(), m_pAllObjects.end(), pObject);
		if (it != m_pAllObjects.end()) m_pAllObjects.erase(it);
	}

	Object* ObjectManager::Find(const UUID id)
	{
		auto iter = std::find_if(m_pAllObjects.begin(), m_pAllObjects.end(),
			[id](Object* pObject) { return id == pObject->GetUUID(); });
		return iter == m_pAllObjects.end() ? nullptr : *iter;
	}

	ObjectManager::~ObjectManager()
	{
		std::for_each(m_pAllObjects.begin(), m_pAllObjects.end(), [](Object* const& pObj) { delete pObj; });
		m_pAllObjects.clear();
	}
}

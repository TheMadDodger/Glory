#include "ObjectManager.h"
#include "Object.h"

namespace Glory
{
	void ObjectManager::Add(Object* pObject)
	{
		m_pAllObjects.push_back(pObject);
	}

	void ObjectManager::Remove(Object* pObject)
	{
		m_pAllObjects.Erase(pObject);
	}

	Object* ObjectManager::Find(const UUID id)
	{
		Object* pObject = nullptr;
		if(!m_pAllObjects.Find([&](Object* pObject) { return id == pObject->GetUUID(); }, pObject)) return nullptr;
		return pObject;
	}

	ObjectManager::~ObjectManager()
	{
		m_pAllObjects.ForEachClear([](Object* const& pObj) { delete pObj; });
	}
}

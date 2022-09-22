#include "Object.h"
#include "GloryContext.h"

namespace Glory
{
	Object::Object()
	{
		APPEND_TYPE(Object);
		GloryContext* pContext = GloryContext::GetContext();
		pContext->m_pAllObjects.push_back(this);
	}

	Object::Object(UUID uuid) : m_ID(uuid)
	{
		APPEND_TYPE(Object);

		GloryContext* pContext = GloryContext::GetContext();
		pContext->m_pAllObjects.push_back(this);
	}

	Object::~Object()
	{
		m_Inheritence.clear();
		GloryContext* pContext = GloryContext::GetContext();
		pContext->m_pAllObjects.Erase(this);
	}

	const UUID& Object::GetUUID() const
	{
		return m_ID;
	}

	const UUID& Object::GetGPUUUID() const
	{
		return GetUUID();
	}

	size_t Object::TypeCount()
	{
		return m_Inheritence.size();
	}

	bool Object::GetType(size_t index, std::type_index& type)
	{
		if (index >= m_Inheritence.size()) return false;
		type = m_Inheritence[index];
		return true;
	}

	Object* Object::FindObject(UUID id)
	{
		GloryContext* pContext = GloryContext::GetContext();
		Object* pObject = nullptr;
		if (!pContext->m_pAllObjects.Find([&](Object* pObject) { return id == pObject->m_ID; }, pObject)) return nullptr;
		return pObject;
	}

	void Object::PushInheritence(const std::type_index& type)
	{
		m_Inheritence.insert(m_Inheritence.begin(), type);
	}
}
#include "Object.h"
#include "GloryContext.h"

namespace Glory
{
	Object::Object() : m_ID(UUID()), m_Name("Unnamed Object")
	{
		APPEND_TYPE(Object);
		GloryContext* pContext = GloryContext::GetContext();
		pContext->m_pAllObjects.push_back(this);
	}

	Object::Object(const std::string& name) : m_ID(UUID()), m_Name(name)
	{
		APPEND_TYPE(Object);
		GloryContext* pContext = GloryContext::GetContext();
		pContext->m_pAllObjects.push_back(this);
	}

	Object::Object(UUID uuid) : m_ID(uuid), m_Name("Unnamed Object")
	{
		APPEND_TYPE(Object);
		GloryContext* pContext = GloryContext::GetContext();
		pContext->m_pAllObjects.push_back(this);
	}

	Object::Object(UUID uuid, const std::string& name) : m_ID(uuid), m_Name(name)
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

	uint32_t Object::TypeCount() const
	{
		return m_Inheritence.size();
	}

	bool Object::GetType(size_t index, std::type_index& type) const
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

	const std::string& Object::Name()
	{
		return m_Name;
	}

	void Object::SetName(const std::string& name)
	{
		m_Name = name;
	}

	void* Object::GetRootDataAddress()
	{
		return (void*)this;
	}

	void Object::PushInheritence(const std::type_index& type)
	{
		m_Inheritence.insert(m_Inheritence.begin(), type);
	}
}
#include "Object.h"
#include "ObjectManager.h"

namespace Glory
{
	Object::Object() : m_ID(UUID()), m_Name("Unnamed Object")
	{
		APPEND_TYPE(Object);
	}

	Object::Object(const std::string& name) : m_ID(UUID()), m_Name(name)
	{
		APPEND_TYPE(Object);
	}

	Object::Object(UUID uuid) : m_ID(uuid), m_Name("Unnamed Object")
	{
		APPEND_TYPE(Object);
	}

	Object::Object(UUID uuid, const std::string& name) : m_ID(uuid), m_Name(name)
	{
		APPEND_TYPE(Object);
	}

	Object::~Object()
	{
		m_Inheritence.clear();
	}

	UUID Object::GetUUID() const
	{
		return m_ID;
	}

	UUID Object::GetGPUUUID() const
	{
		return GetUUID();
	}

	size_t Object::TypeCount() const
	{
		return m_Inheritence.size();
	}

	bool Object::GetType(size_t index, std::type_index& type) const
	{
		if (index >= m_Inheritence.size()) return false;
		type = m_Inheritence[index];
		return true;
	}

	const std::string& Object::Name()
	{
		return m_Name;
	}

	void Object::SetName(const std::string& name)
	{
		m_Name = name;
	}

	void Object::SetName(const std::string_view name)
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
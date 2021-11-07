#include "Object.h"

namespace Glory
{
	Object::Object()
	{
		APPEND_TYPE(Object);
	}

	Object::Object(UUID uuid) : m_ID(uuid)
	{
		APPEND_TYPE(Object);
	}

	Object::~Object()
	{
		m_Inheritence.clear();
	}

	const UUID& Object::GetUUID() const
	{
		return m_ID;
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
}
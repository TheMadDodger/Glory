#include "Object.h"

namespace Glory
{
	Object::Object()
	{
	}

	Object::Object(UUID uuid) : m_ID(uuid)
	{
	}

	Object::~Object()
	{
	}

	const UUID& Object::GetUUID() const
	{
		return m_ID;
	}
}
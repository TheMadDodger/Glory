#include "Object.h"

namespace Glory
{
	Object::Object()
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
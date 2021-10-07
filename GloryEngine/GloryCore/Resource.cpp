#include "Resource.h"

namespace Glory
{
	Resource::Resource()
	{
	}

	Resource::~Resource()
	{
	}

	const UUID& Resource::GetUUID() const
	{
		return m_ID;
	}
}

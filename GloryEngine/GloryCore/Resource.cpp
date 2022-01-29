#include "Resource.h"

namespace Glory
{
	Resource::Resource()
	{
		APPEND_TYPE(Resource);
	}

	Resource::Resource(UUID uuid) : Object(uuid)
	{
		APPEND_TYPE(Resource);
	}

	Resource::~Resource()
	{
	}

	const std::string& Resource::Name()
	{
		return m_Name;
	}
}

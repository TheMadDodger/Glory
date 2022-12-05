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

	Resource::Resource(const std::string& name) : Object(name)
	{
		APPEND_TYPE(Resource);
	}

	Resource::Resource(UUID uuid, const std::string& name) : Object(uuid, name)
	{
		APPEND_TYPE(Resource);
	}

	Resource::~Resource()
	{
	}
}

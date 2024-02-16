#include "Resource.h"
#include "Debug.h"
#include "BinaryStream.h"

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

	void Resource::SetResourceUUID(UUID uuid)
	{
		m_ID = uuid;
	}
}

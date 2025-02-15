#include "Resource.h"
#include "Debug.h"
#include "BinaryStream.h"

namespace Glory
{
	Resource::Resource(): m_IsDirty(true)
	{
		APPEND_TYPE(Resource);
	}

	Resource::Resource(UUID uuid) : Object(uuid), m_IsDirty(true)
	{
		APPEND_TYPE(Resource);
	}

	Resource::Resource(const std::string& name) : Object(name), m_IsDirty(true)
	{
		APPEND_TYPE(Resource);
	}

	Resource::Resource(UUID uuid, const std::string& name) : Object(uuid, name), m_IsDirty(true)
	{
		APPEND_TYPE(Resource);
	}

	Resource::~Resource()
	{
	}

	bool Resource::IsDirty() const
	{
		return m_IsDirty;
	}

	void Resource::SetDirty(bool dirty)
	{
		m_IsDirty = dirty;
	}

	void Resource::SetResourceUUID(UUID uuid)
	{
		m_ID = uuid;
	}
}

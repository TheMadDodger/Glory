#include "Resource.h"
#include "Debug.h"

#include <BinaryStream.h>

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

	Resource::Resource(Resource&& other) noexcept: Object()
	{
		m_Inheritence = std::move(other.m_Inheritence);
		m_ID = other.m_ID;
		m_Name = std::move(other.m_Name);
	}

	Resource& Resource::operator=(Resource&& other) noexcept
	{
		m_Inheritence = std::move(other.m_Inheritence);
		m_ID = other.m_ID;
		m_Name = std::move(other.m_Name);
		++m_DirtyVersion;
		return *this;
	}

	Resource::~Resource()
	{
	}

	bool Resource::IsDirty(uint64_t compare) const
	{
		return compare < m_DirtyVersion;
	}

	void Resource::IncrementDirtyVersion()
	{
		++m_DirtyVersion;
	}

	uint64_t Resource::DirtyVersion() const
	{
		return m_DirtyVersion;
	}

	void Resource::SetDirtyVersion(uint64_t version)
	{
		m_DirtyVersion = version;
	}

	void Resource::SetResourceUUID(UUID uuid)
	{
		m_ID = uuid;
	}
}

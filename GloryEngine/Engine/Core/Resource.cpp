#include "Resource.h"
#include "Debug.h"
#include "BinaryStream.h"

namespace Glory
{
	Resource::Resource() : m_pParent(nullptr)
	{
		APPEND_TYPE(Resource);
	}

	Resource::Resource(UUID uuid) : Object(uuid), m_pParent(nullptr)
	{
		APPEND_TYPE(Resource);
	}

	Resource::Resource(const std::string& name) : Object(name), m_pParent(nullptr)
	{
		APPEND_TYPE(Resource);
	}

	Resource::Resource(UUID uuid, const std::string& name) : Object(uuid, name), m_pParent(nullptr)
	{
		APPEND_TYPE(Resource);
	}

	Resource::~Resource()
	{
		m_pSubresources.clear();
		m_pParent = nullptr;
	}

	const size_t Resource::SubResourceCount() const
	{
		return m_pSubresources.size();
	}

	Resource* Resource::SubresourceFromPath(std::filesystem::path path)
	{
		auto subPathItor = path.begin();
		if (subPathItor == path.end()) return this;
		std::filesystem::path subPath = *subPathItor;
		if (subPath == ".") return this;
		Resource* pSubresource = Subresource(subPath.string());
		if (!pSubresource) return nullptr;
		std::filesystem::path nextPath = path.lexically_relative(subPath);
		return pSubresource->SubresourceFromPath(nextPath);
	}

	void Resource::AddSubresource(Resource* pResource, const std::string& name)
	{
		if (m_NameToSubresourceIndex.find(name) != m_NameToSubresourceIndex.end())
		{
			//m_pEngine->GetDebug().LogError("Could not add Subresource because a Subresource with the same name already exists!");
			return;
		}
		size_t index = m_pSubresources.size();
		m_pSubresources.push_back(pResource);
		m_NameToSubresourceIndex.emplace(name, index);
		pResource->m_pParent = this;
	}

	void Resource::SetResourceUUID(UUID uuid)
	{
		m_ID = uuid;
	}

	Resource* Resource::Subresource(size_t index) const
	{
		if (index >= m_pSubresources.size()) return nullptr;
		return m_pSubresources[index];
	}

	Resource* Resource::Subresource(std::string_view name) const
	{
		auto itor = m_NameToSubresourceIndex.find(name);
		if (itor == m_NameToSubresourceIndex.end())
			return nullptr;
		const size_t index = itor->second;
		return Subresource(index);
	}

	Resource* Resource::ParentResource() const
	{
		return m_pParent;
	}
}

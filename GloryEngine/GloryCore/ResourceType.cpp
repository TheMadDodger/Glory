#include "ResourceType.h"
#include <sstream>

namespace Glory
{
	std::vector<ResourceType> ResourceType::m_ResourceTypes;
	std::unordered_map<std::string, size_t> ResourceType::m_ExtensionToType;
	std::unordered_map<size_t, size_t> ResourceType::m_HashToType;
	std::hash<std::type_index> ResourceType::m_Hasher;

	void ResourceType::RegisterResource(const std::type_index& type, const std::string& extensions)
	{
		size_t typeHash = m_Hasher(type);
		size_t index = m_ResourceTypes.size();
		m_ResourceTypes.push_back(ResourceType(typeHash, extensions));
		m_HashToType[typeHash] = index;
		ReadExtensions(index, extensions);
	}

	size_t ResourceType::GetHash(const std::type_index& type)
	{
		return m_Hasher(type);
	}

	ResourceType* ResourceType::GetResourceType(const std::string& extension)
	{
		if (m_ExtensionToType.find(extension) == m_ExtensionToType.end()) return nullptr;
		size_t index = m_ExtensionToType[extension];
		return &m_ResourceTypes[index];
	}

	ResourceType* ResourceType::GetResourceType(const std::type_index& type)
	{
		return GetResourceType(m_Hasher(type));
	}

	ResourceType* ResourceType::GetResourceType(size_t hash)
	{
		if (m_HashToType.find(hash) == m_HashToType.end()) return nullptr;
		size_t index = m_HashToType[hash];
		return &m_ResourceTypes[index];
	}

	ResourceType::ResourceType(size_t typeHash, const std::string& extensions)
		: m_TypeHash(typeHash), m_Extensions(extensions) {}

	ResourceType::~ResourceType() {}

	size_t ResourceType::Hash() const
	{
		return m_TypeHash;
	}

	const std::string& ResourceType::Extensions() const
	{
		return m_Extensions;
	}

	void ResourceType::ReadExtensions(size_t index, const std::string& extensions)
	{
		std::stringstream stream(extensions);
		std::string next = "";
		while (std::getline(stream, next, ','))
		{
			m_ExtensionToType[next] = index;
		}
	}
}

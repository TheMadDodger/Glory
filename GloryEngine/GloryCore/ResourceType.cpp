#include "ResourceType.h"
#include <sstream>

namespace Glory
{
	BasicTypeData::BasicTypeData(const std::string& name, size_t typeHash, size_t size)
		: m_Name(name), m_TypeHash(typeHash), m_Size(size) {}

	std::vector<ResourceType> ResourceType::m_ResourceTypes;
	std::unordered_map<std::string, size_t> ResourceType::m_ExtensionToType;
	std::unordered_map<size_t, size_t> ResourceType::m_HashToType;

	std::vector<BasicTypeData> ResourceType::m_BasicTypes;
	std::unordered_map<size_t, size_t> ResourceType::m_HashToBasicType;
	std::unordered_map<std::string, size_t> ResourceType::m_NameToBasicType;

	std::hash<std::type_index> ResourceType::m_Hasher;

	bool ResourceType::IsResource(size_t typeHash)
	{
		return m_HashToType.find(typeHash) != m_HashToType.end();
	}

	void ResourceType::RegisterResource(std::type_index type, const std::string& extensions)
	{
		size_t typeHash = m_Hasher(type);
		size_t index = m_ResourceTypes.size();
		m_ResourceTypes.push_back(ResourceType(typeHash, extensions));
		m_HashToType[typeHash] = index;
		ReadExtensions(index, extensions);
	}

	void ResourceType::RegisterType(const std::type_info& type, size_t size)
	{
		size_t hash = GetHash(type);
		size_t index = m_BasicTypes.size();
		std::string name = type.name();
		m_HashToBasicType[hash] = index;
		m_NameToBasicType[name] = index;
		m_BasicTypes.push_back(BasicTypeData(type.name(), hash, size));
	}

	size_t ResourceType::GetHash(std::type_index type)
	{
		return m_Hasher(type);
	}

	ResourceType* ResourceType::GetResourceType(const std::string& extension)
	{
		if (m_ExtensionToType.find(extension) == m_ExtensionToType.end()) return nullptr;
		size_t index = m_ExtensionToType[extension];
		return &m_ResourceTypes[index];
	}

	ResourceType* ResourceType::GetResourceType(std::type_index type)
	{
		return GetResourceType(m_Hasher(type));
	}

	ResourceType* ResourceType::GetResourceType(size_t hash)
	{
		if (m_HashToType.find(hash) == m_HashToType.end()) return nullptr;
		size_t index = m_HashToType[hash];
		return &m_ResourceTypes[index];
	}

	const BasicTypeData* ResourceType::GetBasicTypeData(size_t typeHash)
	{
		if (m_HashToBasicType.find(typeHash) == m_HashToBasicType.end()) return nullptr;
		size_t index = m_HashToBasicType[typeHash];
		return &m_BasicTypes[index];
	}
	
	const BasicTypeData* ResourceType::GetBasicTypeData(const std::string& name)
	{
		if (m_NameToBasicType.find(name) == m_NameToBasicType.end()) return nullptr;
		size_t index = m_NameToBasicType[name];
		return &m_BasicTypes[index];
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

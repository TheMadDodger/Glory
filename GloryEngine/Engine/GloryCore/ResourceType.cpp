#include "ResourceType.h"
#include "GloryContext.h"
#include <sstream>

#define RESOURCE_TYPES Glory::GloryContext::GetResourceTypes()

namespace Glory
{
	BasicTypeData::BasicTypeData(const std::string& name, size_t typeHash, size_t size)
		: m_Name(name), m_TypeHash(typeHash), m_Size(size) {}

	std::hash<std::type_index> ResourceType::m_Hasher;

	bool ResourceType::IsResource(size_t typeHash)
	{
		return RESOURCE_TYPES->m_HashToType.find(typeHash) != RESOURCE_TYPES->m_HashToType.end();
	}

	ResourceType* ResourceType::RegisterResource(std::type_index type, const std::string& extensions)
	{
		size_t typeHash = m_Hasher(type);
		size_t index = RESOURCE_TYPES->m_ResourceTypes.size();
		RESOURCE_TYPES->m_ResourceTypes.push_back(ResourceType(typeHash, extensions));
		RESOURCE_TYPES->m_HashToType[typeHash] = index;
		ReadExtensions(index, extensions);
		return &RESOURCE_TYPES->m_ResourceTypes[index];
	}

	void ResourceType::RegisterType(const std::type_info& type, size_t size)
	{
		size_t hash = GetHash(type);
		size_t index = RESOURCE_TYPES->m_BasicTypes.size();
		std::string name = type.name();
		RESOURCE_TYPES->m_HashToBasicType[hash] = index;
		RESOURCE_TYPES->m_NameToBasicType[name] = index;
		RESOURCE_TYPES->m_BasicTypes.push_back(BasicTypeData(type.name(), hash, size));
	}

	size_t ResourceType::GetHash(std::type_index type)
	{
		return m_Hasher(type);
	}

	ResourceType* ResourceType::GetResourceType(const std::string& extension)
	{
		if (RESOURCE_TYPES->m_ExtensionToType.find(extension) == RESOURCE_TYPES->m_ExtensionToType.end()) return nullptr;
		size_t index = RESOURCE_TYPES->m_ExtensionToType[extension];
		return &RESOURCE_TYPES->m_ResourceTypes[index];
	}

	ResourceType* ResourceType::GetResourceType(std::type_index type)
	{
		return GetResourceType(m_Hasher(type));
	}

	ResourceType* ResourceType::GetResourceType(size_t hash)
	{
		if (RESOURCE_TYPES->m_HashToType.find(hash) == RESOURCE_TYPES->m_HashToType.end()) return nullptr;
		size_t index = RESOURCE_TYPES->m_HashToType[hash];
		return &RESOURCE_TYPES->m_ResourceTypes[index];
	}

	const BasicTypeData* ResourceType::GetBasicTypeData(size_t typeHash)
	{
		if (RESOURCE_TYPES->m_HashToBasicType.find(typeHash) == RESOURCE_TYPES->m_HashToBasicType.end()) return nullptr;
		size_t index = RESOURCE_TYPES->m_HashToBasicType[typeHash];
		return &RESOURCE_TYPES->m_BasicTypes[index];
	}

	const BasicTypeData* ResourceType::GetBasicTypeData(const std::string& name)
	{
		if (RESOURCE_TYPES->m_NameToBasicType.find(name) == RESOURCE_TYPES->m_NameToBasicType.end()) return nullptr;
		size_t index = RESOURCE_TYPES->m_NameToBasicType[name];
		return &RESOURCE_TYPES->m_BasicTypes[index];
	}

	size_t ResourceType::SubTypeCount(ResourceType* pResourceType)
	{
		return pResourceType->m_SubTypes.size();
	}

	ResourceType* ResourceType::GetSubType(ResourceType* pResourceType, size_t index)
	{
		size_t subTypeHash = GetSubTypeHash(pResourceType, index);
		if (subTypeHash == 0) return nullptr;
		return GetResourceType(subTypeHash);
	}

	size_t ResourceType::GetSubTypeHash(ResourceType* pResourceType, size_t index)
	{
		if (index >= pResourceType->m_SubTypes.size()) return 0;
		return pResourceType->m_SubTypes[index];
	}

	size_t ResourceType::GetAllResourceTypesThatHaveSubType(size_t hash, std::vector<ResourceType*>& out)
	{
		size_t result = 0;
		for (size_t i = 0; i < RESOURCE_TYPES->m_ResourceTypes.size(); i++)
		{
			if (std::find(RESOURCE_TYPES->m_ResourceTypes[i].m_SubTypes.begin(), RESOURCE_TYPES->m_ResourceTypes[i].m_SubTypes.end(), hash)
				== RESOURCE_TYPES->m_ResourceTypes[i].m_SubTypes.end()) continue;
			out.push_back(&RESOURCE_TYPES->m_ResourceTypes[i]);
			++result;
		}
		return result;
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
		if (extensions.empty()) return;
		std::stringstream stream(extensions);
		std::string next = "";
		while (std::getline(stream, next, ','))
		{
			RESOURCE_TYPES->m_ExtensionToType[next] = index;
		}
	}
	ResourceTypes::ResourceTypes()
	{
	}
	ResourceTypes::~ResourceTypes()
	{
	}
}

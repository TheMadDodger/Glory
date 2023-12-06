#include "ResourceType.h"

#include <sstream>
#include <sstream>
#include <Hash.h>

namespace Glory
{
	BasicTypeData::BasicTypeData(const std::string& name, uint32_t typeHash, size_t size)
		: m_Name(name), m_TypeHash(typeHash), m_Size(size) {}

	bool ResourceTypes::IsResource(uint32_t typeHash)
	{
		return m_HashToType.find(typeHash) != m_HashToType.end();
	}

	ResourceType* ResourceTypes::RegisterResource(std::type_index type, const std::string& extensions, ResourceFactory* pFactory)
	{
		uint32_t typeHash = GetHash(type);
		size_t index = m_ResourceTypes.size();
		m_ResourceTypes.push_back(ResourceType(typeHash, extensions, type.name(), pFactory));
		m_HashToType[typeHash] = index;
		ReadExtensions(index, extensions);
		return &m_ResourceTypes[index];
	}

	void ResourceTypes::RegisterType(const std::type_info& type, size_t size)
	{
		uint32_t hash = GetHash(type);
		size_t index = m_BasicTypes.size();
		std::string name = type.name();
		m_HashToBasicType[hash] = index;
		m_NameToBasicType[name] = index;
		m_BasicTypes.push_back(BasicTypeData(type.name(), hash, size));
	}

	uint32_t ResourceTypes::GetHash(std::type_index type)
	{
		return Hashing::Hash(type.name());
	}

	ResourceType* ResourceTypes::GetResourceType(const std::string& extension)
	{
		if (m_ExtensionToType.find(extension) == m_ExtensionToType.end()) return nullptr;
		size_t index = m_ExtensionToType[extension];
		return &m_ResourceTypes[index];
	}

	ResourceType* ResourceTypes::GetResourceType(std::type_index type)
	{
		return GetResourceType(Hashing::Hash(type.name()));
	}

	ResourceType* ResourceTypes::GetResourceType(uint32_t hash)
	{
		if (m_HashToType.find(hash) == m_HashToType.end()) return nullptr;
		size_t index = m_HashToType[hash];
		return &m_ResourceTypes[index];
	}

	const BasicTypeData* ResourceTypes::GetBasicTypeData(uint32_t typeHash)
	{
		if (m_HashToBasicType.find(typeHash) == m_HashToBasicType.end()) return nullptr;
		size_t index = m_HashToBasicType[typeHash];
		return &m_BasicTypes[index];
	}

	const BasicTypeData* ResourceTypes::GetBasicTypeData(const std::string& name)
	{
		if (m_NameToBasicType.find(name) == m_NameToBasicType.end()) return nullptr;
		size_t index = m_NameToBasicType[name];
		return &m_BasicTypes[index];
	}

	size_t ResourceTypes::SubTypeCount(const ResourceType* pResourceType)
	{
		return pResourceType->m_SubTypes.size();
	}

	ResourceType* ResourceTypes::GetSubType(const ResourceType* pResourceType, size_t index)
	{
		uint32_t subTypeHash = GetSubTypeHash(pResourceType, index);
		if (subTypeHash == 0) return nullptr;
		return GetResourceType(subTypeHash);
	}

	uint32_t ResourceTypes::GetSubTypeHash(const ResourceType* pResourceType, size_t index)
	{
		if (index >= pResourceType->m_SubTypes.size()) return 0;
		return pResourceType->m_SubTypes[index];
	}

	size_t ResourceTypes::GetAllResourceTypesThatHaveSubType(uint32_t hash, std::vector<ResourceType*>& out)
	{
		size_t result = 0;
		for (size_t i = 0; i < m_ResourceTypes.size(); i++)
		{
			if (std::find(m_ResourceTypes[i].m_SubTypes.begin(), m_ResourceTypes[i].m_SubTypes.end(), hash)
				== m_ResourceTypes[i].m_SubTypes.end()) continue;
			out.push_back(&m_ResourceTypes[i]);
			++result;
		}
		return result;
	}

	bool ResourceTypes::IsScene(const std::string& ext)
	{
		/* FIXME: Should be handled by the scene module */
		return ext == ".gscene";
	}

	ResourceType::ResourceType(uint32_t typeHash, const std::string& extensions, const char* name, ResourceFactory* pFactory)
		: m_TypeHash(typeHash), m_Extensions(extensions), m_FullName(name), m_pFactory(pFactory)
	{
		static constexpr std::string_view classNamespaceName = "class Glory::";
		const size_t classIndex = m_FullName.find(classNamespaceName);
		m_Name = classIndex == std::string::npos ? m_FullName : m_FullName.substr(classNamespaceName.length());
		const size_t dataIndex = m_Name.find("Data");
		m_Name = m_Name.substr(0, dataIndex);
	}

	ResourceType::~ResourceType()
	{
		m_pFactory = nullptr;
	}

	uint32_t ResourceType::Hash() const
	{
		return m_TypeHash;
	}

	const std::string& ResourceType::Extensions() const
	{
		return m_Extensions;
	}

	const std::string& ResourceType::FullName() const
	{
		return m_FullName;
	}

	const std::string& ResourceType::Name() const
	{
		return m_Name;
	}

	Resource* ResourceType::Create() const
	{
		return m_pFactory->Create();
	}

	Resource* ResourceType::Create(const UUID uuid, const std::string& name) const
	{
		return m_pFactory->Create(uuid, name);
	}

	void ResourceType::ReadExtensions(size_t index, const std::string& extensions)
	{
		if (extensions.empty()) return;
		std::stringstream stream(extensions);
		std::string next = "";
		while (std::getline(stream, next, ','))
		{
			m_ExtensionToType[next] = index;
		}
	}

	ResourceTypes::ResourceTypes()
	{
	}

	ResourceTypes::~ResourceTypes()
	{
		for (size_t i = 0; i < m_ResourceTypes.size(); ++i)
		{
			delete m_ResourceTypes[i].m_pFactory;
		}

		m_ResourceTypes.clear();
		m_ExtensionToType.clear();
		m_HashToType.clear();
		m_BasicTypes.clear();
		m_HashToBasicType.clear();
		m_NameToBasicType.clear();
	}
}

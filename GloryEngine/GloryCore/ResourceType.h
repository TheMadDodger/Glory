#pragma once
#include <string>
#include <typeindex>
#include <vector>
#include <unordered_map>

namespace Glory
{
	struct BasicTypeData
	{
		BasicTypeData(const std::string& name, size_t typeHash, size_t size);

		const std::string m_Name;
		size_t m_TypeHash;
		size_t m_Size;
	};

	class ResourceType
	{
	public:
		template<class T>
		static void RegisterResource(const std::string& extensions)
		{
			ResourceType* pResourceType = RegisterResource(typeid(T), extensions);
			T t = T();
			for (size_t i = 0; i < t.TypeCount(); i++)
			{
				std::type_index type = typeid(Object);
				if (!t.GetType(i, type)) continue;
				size_t hash = m_Hasher(type);
				pResourceType->m_SubTypes.push_back(hash);
			}
		}

		template<typename T>
		static void RegisterType()
		{
			RegisterType(typeid(T), sizeof(T));
		}

		template<typename T>
		static size_t GetHash()
		{
			return GetHash(typeid(T));
		}

		template<typename T>
		static ResourceType* GetResourceType()
		{
			return GetResourceType(typeid(T));
		}

		static bool IsResource(size_t typeHash);
		static ResourceType* RegisterResource(std::type_index type, const std::string& extensions);
		static void RegisterType(const std::type_info& type, size_t size);
		static size_t GetHash(std::type_index type);
		static ResourceType* GetResourceType(const std::string& extension);
		static ResourceType* GetResourceType(std::type_index type);
		static ResourceType* GetResourceType(size_t hash);
		static const BasicTypeData* GetBasicTypeData(size_t typeHash);
		static const BasicTypeData* GetBasicTypeData(const std::string& name);

		static size_t SubTypeCount(ResourceType* pResourceType);
		static ResourceType* GetSubType(ResourceType* pResourceType, size_t index);
		static size_t GetAllResourceTypesThatHaveSubType(size_t hash, std::vector<ResourceType*>& out);

	public:
		virtual ~ResourceType();
		size_t Hash() const;
		const std::string& Extensions() const;

	private:
		ResourceType(size_t typeHash, const std::string& extensions);
		const ResourceType operator=(const ResourceType&) = delete;

	private:
		static void ReadExtensions(size_t index, const std::string& extensions);

	private:
		static std::vector<ResourceType> m_ResourceTypes;
		static std::unordered_map<std::string, size_t> m_ExtensionToType;
		static std::unordered_map<size_t, size_t> m_HashToType;

		static std::vector<BasicTypeData> m_BasicTypes;
		static std::unordered_map<size_t, size_t> m_HashToBasicType;
		static std::unordered_map<std::string, size_t> m_NameToBasicType;
		
		static std::hash<std::type_index> m_Hasher;

		const size_t m_TypeHash;
		const std::string m_Extensions;
		std::vector<size_t> m_SubTypes;
	};
}

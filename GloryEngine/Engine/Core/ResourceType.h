#pragma once
#include <string>
#include <string_view>
#include <typeindex>
#include <vector>
#include <unordered_map>

namespace Glory
{
	struct BasicTypeData
	{
		BasicTypeData(const std::string& name, uint32_t typeHash, size_t size);

		const std::string m_Name;
		uint32_t m_TypeHash;
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
				uint32_t hash = GetHash(type);
				pResourceType->m_SubTypes.push_back(hash);
			}
		}

		template<typename T>
		static void RegisterType()
		{
			RegisterType(typeid(T), sizeof(T));
		}

		template<typename T>
		static uint32_t GetHash()
		{
			return GetHash(typeid(T));
		}

		template<typename T>
		static ResourceType* GetResourceType()
		{
			return GetResourceType(typeid(T));
		}

		static bool IsResource(uint32_t typeHash);
		static ResourceType* RegisterResource(std::type_index type, const std::string& extensions);
		static void RegisterType(const std::type_info& type, size_t size);
		static uint32_t GetHash(std::type_index type);
		static ResourceType* GetResourceType(const std::string& extension);
		static ResourceType* GetResourceType(std::type_index type);
		static ResourceType* GetResourceType(uint32_t hash);
		static const BasicTypeData* GetBasicTypeData(uint32_t typeHash);
		static const BasicTypeData* GetBasicTypeData(const std::string& name);

		static size_t SubTypeCount(ResourceType* pResourceType);
		static ResourceType* GetSubType(ResourceType* pResourceType, size_t index);
		static uint32_t GetSubTypeHash(ResourceType* pResourceType, size_t index);
		static size_t GetAllResourceTypesThatHaveSubType(uint32_t hash, std::vector<ResourceType*>& out);

		static bool IsScene(const std::string& ext);

	public:
		virtual ~ResourceType();
		uint32_t Hash() const;
		const std::string& Extensions() const;
		const std::string& Name() const;

	private:
		ResourceType(uint32_t typeHash, const std::string& extensions, const char* name);
		const ResourceType operator=(const ResourceType&) = delete;

	private:
		static void ReadExtensions(size_t index, const std::string& extensions);

	private:
		const uint32_t m_TypeHash;
		const std::string m_Extensions;
		const std::string m_Name;
		std::vector<uint32_t> m_SubTypes;
	};

	class ResourceTypes
	{
	public:
		ResourceTypes();
		virtual ~ResourceTypes();

	private:
		friend class ResourceType;
		std::vector<ResourceType> m_ResourceTypes;
		std::unordered_map<std::string, size_t> m_ExtensionToType;
		std::unordered_map<uint32_t, size_t> m_HashToType;

		std::vector<BasicTypeData> m_BasicTypes;
		std::unordered_map<uint32_t, size_t> m_HashToBasicType;
		std::unordered_map<std::string, size_t> m_NameToBasicType;
	};
}

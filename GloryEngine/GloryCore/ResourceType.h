#pragma once
#include <string>
#include <typeindex>
#include <vector>
#include <unordered_map>

namespace Glory
{
	class ResourceType
	{
	public:
		template<class T>
		static void RegisterResource(const std::string& extensions)
		{
			RegisterResource(typeid(T), extensions);
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

		static void RegisterResource(const std::type_index& type, const std::string& extensions);
		static size_t GetHash(const std::type_index& type);
		static ResourceType* GetResourceType(const std::string& extension);
		static ResourceType* GetResourceType(const std::type_index& type);
		static ResourceType* GetResourceType(size_t hash);

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
		static std::hash<std::type_index> m_Hasher;

		const size_t m_TypeHash;
		const std::string m_Extensions;
	};
}

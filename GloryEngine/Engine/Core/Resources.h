#pragma once
#include "ResourceManager.h"
#include "ResourceType.h"

namespace Glory
{
	class Resources
	{
	public:
		template<class T>
		void Register()
		{
			const uint32_t hash = ResourceType::GetHash<T>();
			BaseResourceManager* pManager = new ResourceManager<T>();
			Register(hash, pManager);
			ResourceType::RegisterResource<T>();
		}

		template<class T>
		ResourceManager<T>* Manager()
		{
			const uint32_t hash = ResourceType::GetHash<T>();
			BaseResourceManager* pManager = Manager(hash);
			if (!pManager) return nullptr;
			return static_cast<ResourceManager<T>*>(pManager);
		}

		bool Add(Resource* pResource);

	private:
		void Register(uint32_t hash, BaseResourceManager* pManager);
		BaseResourceManager* Manager(uint32_t hash);

	private:
		~Resources();

	private:
		friend class Engine;
		std::vector<uint32_t> m_Types;
		std::vector<BaseResourceManager*> m_pManagers;
	};
}

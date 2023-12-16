#pragma once
#include "ResourceManager.h"
#include "ResourceType.h"

#include "Engine.h"

namespace Glory
{
	class Resources
	{
	public:
		template<class T>
		void Register()
		{
			const uint32_t hash = ResourceTypes::GetHash<T>();
			BaseResourceManager* pManager = new ResourceManager<T>(this);
			Register(hash, pManager);
			m_pEngine->GetResourceTypes().RegisterResource<T>();
		}

		template<class T>
		ResourceManager<T>* Manager()
		{
			const uint32_t hash = ResourceTypes::GetHash<T>();
			BaseResourceManager* pManager = Manager(hash);
			if (!pManager) return nullptr;
			return static_cast<ResourceManager<T>*>(pManager);
		}

		BaseResourceManager* Manager(uint32_t hash);

		bool Add(Resource* pResource);

		~Resources();

	private:
		void Register(uint32_t hash, BaseResourceManager* pManager);

	private:
		Resources(Engine* pEngine);

	private:
		friend class Engine;
		Engine* m_pEngine;
		std::vector<uint32_t> m_Types;
		std::vector<BaseResourceManager*> m_pManagers;
	};
}

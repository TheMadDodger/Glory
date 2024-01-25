#include "Resources.h"

#include <algorithm>

namespace Glory
{
	bool Resources::Add(Resource* pResource)
	{
		for (size_t i = 0; i < pResource->TypeCount(); ++i)
		{
			std::type_index type = typeid(Resource);
			if (!pResource->GetType(i, type)) continue;
			const uint32_t hash = ResourceTypes::GetHash(type);
			BaseResourceManager* pManager = Manager(hash);
			if (!pManager) continue;
			pManager->Add(pResource);
			return true;
		}
		return false;
	}

	bool Resources::IsLoaded(UUID uuid) const
	{
		for (size_t i = 0; i < m_pManagers.size(); ++i)
		{
			if (!m_pManagers[i]->IsLoaded(uuid))
				continue;
			return true;
		}
		return false;
	}

	void Resources::Register(uint32_t hash, BaseResourceManager* pManager)
	{
		m_Types.push_back(hash);
		m_pManagers.push_back(pManager);
	}

	BaseResourceManager* Resources::Manager(uint32_t hash)
	{
		const auto itor = std::find(m_Types.begin(), m_Types.end(), hash);
		if (itor == m_Types.end()) return nullptr;
		const size_t index = itor - m_Types.begin();
		return m_pManagers[index];
	}

	Resources::Resources(Engine* pEngine): m_pEngine(pEngine) {}

	Resources::~Resources()
	{
		for (size_t i = 0; i < m_pManagers.size(); ++i)
		{
			delete m_pManagers[i];
		}
		m_Types.clear();
		m_pManagers.clear();
	}
}

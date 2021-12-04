#include "EntitySystems.h"
#include "Registry.h"

namespace Glory
{
	EntitySystems::EntitySystems()// : m_pJobPool(Jobs::JobManager::Run<int>())
	{
	}

	EntitySystems::~EntitySystems()
	{
		std::for_each(m_pEntitySystems.begin(), m_pEntitySystems.end(), [](EntitySystem* pSystem) { delete pSystem; });
		m_pEntitySystems.clear();
	}

	bool EntitySystems::IsUpdating()
	{
		//bool isUpdating = m_pJobPool->HasTasksInQueue();
		return false;
	}

	void EntitySystems::OnComponentAdded(Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData)
	{
		for (size_t i = 0; i < m_pEntitySystems.size(); i++)
		{
			EntitySystem* pSystem = m_pEntitySystems[i];
			if (pSystem->m_ComponentType != pComponentData->GetType()) continue;
			pSystem->ComponentAdded(pRegisrty, entity, pComponentData);
		}
	}

	void EntitySystems::OnComponentRemoved(Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData)
	{
		for (size_t i = 0; i < m_pEntitySystems.size(); i++)
		{
			EntitySystem* pSystem = m_pEntitySystems[i];
			if (pSystem->m_ComponentType != pComponentData->GetType()) continue;
			pSystem->ComponentRemoved(pRegisrty, entity, pComponentData);
		}
	}

	void EntitySystems::OnUpdate()
	{
		//m_pJobPool->StartQueue();
		//for (size_t i = 0; i < m_pEntitySystems.size(); i++)
		//{
		//	EntitySystem* pSystem = m_pEntitySystems[i];
		//	m_pJobPool->QueueJob([&]()
		//	{
		//		pSystem->m_pRegistry->ForEach(pSystem->m_ComponentType,
		//			[pSystem](Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData)
		//			{
		//				pSystem->Update(pRegisrty, entity, pComponentData);
		//			});
		//		return 0;
		//	});
		//}
		//m_pJobPool->EndQueue();

		for (size_t i = 0; i < m_pEntitySystems.size(); i++)
		{
			EntitySystem* pSystem = m_pEntitySystems[i];
			pSystem->m_pRegistry->ForEach(pSystem->m_ComponentType,
				[pSystem](Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData)
				{
					pSystem->Update(pRegisrty, entity, pComponentData);
				});
		}
	}

	void EntitySystems::OnDraw()
	{
		//m_pJobPool->StartQueue();
		//for (size_t i = 0; i < m_pEntitySystems.size(); i++)
		//{
		//	EntitySystem* pSystem = m_pEntitySystems[i];
		//	m_pJobPool->QueueJob([&]()
		//		{
		//			pSystem->m_pRegistry->ForEach(pSystem->m_ComponentType,
		//				[pSystem](Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData)
		//				{
		//					pSystem->Draw(pRegisrty, entity, pComponentData);
		//				});
		//			return 0;
		//		});
		//}
		//m_pJobPool->EndQueue();

		for (size_t i = 0; i < m_pEntitySystems.size(); i++)
		{
			EntitySystem* pSystem = m_pEntitySystems[i];
			pSystem->m_pRegistry->ForEach(pSystem->m_ComponentType,
				[pSystem](Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData)
				{
					pSystem->Draw(pRegisrty, entity, pComponentData);
				});
		}
	}
}
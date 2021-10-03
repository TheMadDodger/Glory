#include "EntitySystems.h"
#include "Registry.h"

namespace Glory
{
	EntitySystems::EntitySystems() : m_pJobPool(Jobs::JobManager::Run<int>())
	{
	}

	EntitySystems::~EntitySystems()
	{
		std::for_each(m_pEntitySystems.begin(), m_pEntitySystems.end(), [](EntitySystem* pSystem) { delete pSystem; });
		m_pEntitySystems.clear();
	}

	void EntitySystems::OnUpdate()
	{
		m_pJobPool->StartQueue();
		std::for_each(m_pEntitySystems.begin(), m_pEntitySystems.end(),
			[&](EntitySystem* pSystem)
			{
				m_pJobPool->QueueJob([&]()
					{
						pSystem->m_pRegistry->ForEach(pSystem->m_ComponentType,
							[pSystem](Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData)
							{
								pSystem->Update(pRegisrty, entity, pComponentData);
							});
						return 0;
					});
			}
		);
		m_pJobPool->EndQueue();
	}

	void EntitySystems::OnDraw()
	{
		m_pJobPool->StartQueue();
		std::for_each(m_pEntitySystems.begin(), m_pEntitySystems.end(),
			[&](EntitySystem* pSystem)
			{
				m_pJobPool->QueueJob([&]()
					{
						pSystem->m_pRegistry->ForEach(pSystem->m_ComponentType,
							[pSystem](Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData)
							{
								pSystem->Draw(pRegisrty, entity, pComponentData);
							});
						return 0;
					});
			}
		);
		m_pJobPool->EndQueue();
	}
}
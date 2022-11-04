//#include "EntitySystems.h"
//#include "Registry.h"
//
//namespace Glory
//{
//	EntitySystems::EntitySystems()// : m_pJobPool(Jobs::JobManager::Run<int>())
//	{
//	}
//
//	EntitySystems::~EntitySystems()
//	{
//		std::for_each(m_pEntitySystems.begin(), m_pEntitySystems.end(), [](EntitySystem* pSystem) { delete pSystem; });
//		m_pEntitySystems.clear();
//	}
//
//	bool EntitySystems::IsUpdating()
//	{
//		//bool isUpdating = m_pJobPool->HasTasksInQueue();
//		return false;
//	}
//
//	void EntitySystems::OnComponentAdded(Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData)
//	{
//		for (size_t i = 0; i < m_pEntitySystems.size(); i++)
//		{
//			EntitySystem* pSystem = m_pEntitySystems[i];
//			if (pSystem->m_ComponentType != pComponentData->GetType()) continue;
//			pSystem->ComponentAdded(pRegisrty, entity, pComponentData);
//		}
//	}
//
//	void EntitySystems::OnComponentRemoved(Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData)
//	{
//		for (size_t i = 0; i < m_pEntitySystems.size(); i++)
//		{
//			EntitySystem* pSystem = m_pEntitySystems[i];
//			if (pSystem->m_ComponentType != pComponentData->GetType()) continue;
//			pSystem->ComponentRemoved(pRegisrty, entity, pComponentData);
//		}
//	}
//
//	void EntitySystems::OnUpdate()
//	{
//		//m_pJobPool->StartQueue();
//		//for (size_t i = 0; i < m_pEntitySystems.size(); i++)
//		//{
//		//	EntitySystem* pSystem = m_pEntitySystems[i];
//		//	m_pJobPool->QueueJob([&]()
//		//	{
//		//		pSystem->m_pRegistry->ForEach(pSystem->m_ComponentType,
//		//			[pSystem](Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData)
//		//			{
//		//				pSystem->Update(pRegisrty, entity, pComponentData);
//		//			});
//		//		return 0;
//		//	});
//		//}
//		//m_pJobPool->EndQueue();
//
//		for (size_t i = 0; i < m_pEntitySystems.size(); i++)
//		{
//			EntitySystem* pSystem = m_pEntitySystems[i];
//			pSystem->m_pRegistry->ForEach(pSystem->m_ComponentType,
//				[pSystem](Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData)
//				{
//					pSystem->Update(pRegisrty, entity, pComponentData);
//				});
//		}
//	}
//
//	void EntitySystems::OnDraw()
//	{
//		//m_pJobPool->StartQueue();
//		//for (size_t i = 0; i < m_pEntitySystems.size(); i++)
//		//{
//		//	EntitySystem* pSystem = m_pEntitySystems[i];
//		//	m_pJobPool->QueueJob([&]()
//		//		{
//		//			pSystem->m_pRegistry->ForEach(pSystem->m_ComponentType,
//		//				[pSystem](Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData)
//		//				{
//		//					pSystem->Draw(pRegisrty, entity, pComponentData);
//		//				});
//		//			return 0;
//		//		});
//		//}
//		//m_pJobPool->EndQueue();
//
//		for (size_t i = 0; i < m_pEntitySystems.size(); i++)
//		{
//			EntitySystem* pSystem = m_pEntitySystems[i];
//			pSystem->m_pRegistry->ForEach(pSystem->m_ComponentType,
//				[pSystem](Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData)
//				{
//					pSystem->Draw(pRegisrty, entity, pComponentData);
//				});
//		}
//	}
//
//	std::string EntitySystems::AcquireSerializedProperties(EntityComponentData* pComponentData, std::vector<SerializedProperty*>& properties)
//	{
//		auto it = std::find_if(m_pEntitySystems.begin(), m_pEntitySystems.end(), [&](EntitySystem* pSystem)
//		{
//			return pSystem->m_ComponentType == pComponentData->GetType();
//		});
//
//		if (it == m_pEntitySystems.end()) return "Unknown Component";
//		EntitySystem* pSystem = *it;
//		return pSystem->AcquireSerializedProperties(pComponentData->GetComponentUUID(), pComponentData, properties);
//	}
//
//	bool EntitySystems::CreateComponent(EntityID entity, std::type_index type, UUID uuid)
//	{
//		return CreateComponent(entity, ResourceType::GetHash(type), uuid);
//	}
//
//	bool EntitySystems::CreateComponent(EntityID entity, size_t typeHash, UUID uuid)
//	{
//		auto it = std::find_if(m_pEntitySystems.begin(), m_pEntitySystems.end(), [&](EntitySystem* pSystem)
//		{
//			size_t componentTypeHash = ResourceType::GetHash(pSystem->m_ComponentType);
//			return componentTypeHash == typeHash;
//		});
//
//		if (it == m_pEntitySystems.end()) return false;
//		EntitySystem* pSystem = *it;
//		pSystem->CreateComponent(entity, uuid);
//		return true;
//	}
//
//	size_t EntitySystems::SystemCount()
//	{
//		return m_pEntitySystems.size();
//	}
//
//	EntitySystem* EntitySystems::GetSystem(size_t index)
//	{
//		return m_pEntitySystems[index];
//	}
//}

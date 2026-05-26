#include "PrefabData.h"
#include "GScene.h"
#include "SceneManager.h"
#include "IEngine.h"

#include <yaml-cpp/yaml.h>
#include <NodeRef.h>
#include <TypeData.h>

namespace Glory
{
	PrefabData::PrefabData()
	{
		APPEND_TYPE(PrefabData);
		m_Registry.DisableCalls();
	}

	PrefabData* PrefabData::CreateFromEntity(GScene* pScene, Utils::ECS::EntityID entity)
	{
		PrefabData* pPrefab = new PrefabData();
		pScene->Manager()->GetRegistryFactory().PopulateRegisry(pPrefab->GetRegistry());
		pPrefab->SetName(pScene->EntityName(entity));
		CopyEntity(pPrefab, pScene, entity, 0);
		return pPrefab;
	}

	void PrefabData::CopyEntity(PrefabData* pPrefab, GScene* pScene, Utils::ECS::EntityID entity, Utils::ECS::EntityID parent)
	{
		const UUID uuid = pScene->GetEntityUUID(entity);
		Utils::ECS::EntityRegistry& registry = pScene->GetRegistry();
		const Utils::ECS::EntityID newEntity = registry.CopyEntityToOtherRegistry(entity, parent, &pPrefab->m_Registry);
		pPrefab->m_UUIds.emplace(newEntity, uuid);
		pPrefab->m_Ids.emplace(uuid, newEntity);
		pPrefab->m_Names.emplace(newEntity, pScene->EntityName(entity));
		for (size_t i = 0; i < registry.ChildCount(entity); ++i)
		{
			const Utils::ECS::EntityID child = registry.Child(entity, i);
			CopyEntity(pPrefab, pScene, child, newEntity);
		}
	}

	void PrefabData::References(IEngine*, std::vector<UUID>& references) const
	{
		m_Registry.GetReferences(references);
	}
}

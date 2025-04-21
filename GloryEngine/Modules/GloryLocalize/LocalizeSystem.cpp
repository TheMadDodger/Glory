#include "LocalizeSystem.h"
#include "Localize.h"
#include "LocalizeModule.h"

#include <GScene.h>
#include <SceneManager.h>
#include <Engine.h>

#include <TypeView.h>
#include <EntityRegistry.h>

namespace Glory
{
	void LocalizeSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, Localize& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		LocalizeModule* pModule = pEngine->GetOptionalModule<LocalizeModule>();
		for (const auto& stringTable : pComponent.m_StringTables)
		{
			pModule->LoadStringTable(stringTable.m_STReference.AssetUUID());
		}
	}

	void LocalizeSystem::OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, Localize& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		LocalizeModule* pModule = pEngine->GetOptionalModule<LocalizeModule>();
		for (const auto& stringTable : pComponent.m_StringTables)
		{
			pModule->UnloadStringTable(stringTable.m_STReference.AssetUUID());
		}
	}

	void LocalizeSystem::GetReferences(const Utils::ECS::BaseTypeView* pTypeView, std::vector<UUID>& references)
	{
		for (size_t i = 0; i < pTypeView->Size(); ++i)
		{
			const Localize* pLocalizeComponent = static_cast<const Localize*>(pTypeView->GetComponentAddressFromIndex(i));
			for (const auto& stringTable : pLocalizeComponent->m_StringTables)
			{
				const UUID id = stringTable.m_STReference.AssetUUID();
				if (!id) continue;
				references.push_back(id);
			}
		}
	}
}

#include "LocalizeSystem.h"
#include "Localize.h"
#include "LocalizeModule.h"

#include <GScene.h>
#include <SceneManager.h>
#include <Engine.h>
#include <Debug.h>

#include <TypeView.h>
#include <EntityRegistry.h>
#include <Components.h>

namespace Glory
{
	void StringTableLoaderSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, StringTableLoader& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		LocalizeModule* pModule = pEngine->GetOptionalModule<LocalizeModule>();
		for (const auto& stringTable : pComponent.m_StringTables)
		{
			pModule->LoadStringTable(stringTable.m_STReference.AssetUUID());
		}
	}

	void StringTableLoaderSystem::OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, StringTableLoader& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		LocalizeModule* pModule = pEngine->GetOptionalModule<LocalizeModule>();
		for (const auto& stringTable : pComponent.m_StringTables)
		{
			pModule->UnloadStringTable(stringTable.m_STReference.AssetUUID());
		}
	}

	void StringTableLoaderSystem::GetReferences(const Utils::ECS::BaseTypeView* pTypeView, std::vector<UUID>& references)
	{
		for (size_t i = 0; i < pTypeView->Size(); ++i)
		{
			const StringTableLoader* pLocalizeComponent = static_cast<const StringTableLoader*>(pTypeView->GetComponentAddressFromIndex(i));
			for (const auto& stringTable : pLocalizeComponent->m_StringTables)
			{
				const UUID id = stringTable.m_STReference.AssetUUID();
				if (!id) continue;
				references.push_back(id);
			}
		}
	}

	void LocalizeSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, Localize& pComponent)
	{
		if (pRegistry->HasComponent<TextComponent>(entity)) return;
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		pEngine->GetDebug().LogError("Localize component requires a TextComponent on the entity");
	}

	void LocalizeSystem::OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, Localize& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		LocalizeModule* pModule = pEngine->GetOptionalModule<LocalizeModule>();
		if (!pRegistry->HasComponent<TextComponent>(entity)) return;
		TextComponent& text = pRegistry->GetComponent<TextComponent>(entity);
		const std::string_view fullTerm = pComponent.m_Term;
		const size_t firstDot = fullTerm.find('.');
		if (firstDot == std::string::npos) return;
		const std::string_view tableName = fullTerm.substr(0, firstDot);
		const std::string_view term = fullTerm.substr(firstDot + 1);
		text.m_Dirty |= pModule->FindString(tableName, term, text.m_Text);
	}
}

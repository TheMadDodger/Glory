#include "LocalizeModule.h"
#include "StringTable.h"
#include "Localize.h"
#include "LocalizeSystem.h"

#include <Engine.h>
#include <AssetManager.h>
#include <SceneManager.h>

#include <EntityRegistry.h>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(LocalizeModule);

	LocalizeModule::LocalizeModule()
	{
	}

	LocalizeModule::~LocalizeModule()
	{
	}

	const std::type_info& LocalizeModule::GetModuleType()
	{
		return typeid(LocalizeModule);
	}

	void LocalizeModule::LoadStringTable(UUID tableID)
	{
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(tableID);
		if (!pResource) return;

		StringTable* pTable = static_cast<StringTable*>(pResource);
		for (auto iter = pTable->Begin(); iter != pTable->End(); ++iter)
		{
			m_LoadedStrings.erase(iter->first);
		}

		for (auto iter = pTable->Begin(); iter != pTable->End(); ++iter)
		{
			m_LoadedStrings.emplace(iter->first, iter->second);
		}
	}

	void LocalizeModule::UnloadStringTable(UUID tableID)
	{
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(tableID);
		if (!pResource) return;

		StringTable* pTable = static_cast<StringTable*>(pResource);
		for (auto iter = pTable->Begin(); iter != pTable->End(); ++iter)
		{
			m_LoadedStrings.erase(iter->first);
		}
	}

	void LocalizeModule::Clear()
	{
		m_LoadedStrings.clear();
	}

	void LocalizeModule::Initialize()
	{
		Reflect::SetReflectInstance(&m_pEngine->Reflection());
		m_pEngine->GetResourceTypes().RegisterResource<StringTable>("");

		Reflect::RegisterType<StringTableRef>();
		Reflect::RegisterType<Localize>();

		Utils::ECS::ComponentTypes* pComponentTypes = m_pEngine->GetSceneManager()->ComponentTypesInstance();
		pComponentTypes->RegisterComponent<Localize>();

		pComponentTypes->RegisterInvokaction<Localize>(Utils::ECS::InvocationType::OnValidate, LocalizeSystem::OnValidate);
		pComponentTypes->RegisterInvokaction<Localize>(Utils::ECS::InvocationType::OnRemove, LocalizeSystem::OnStop);
		pComponentTypes->RegisterInvokaction<Localize>(Utils::ECS::InvocationType::Stop, LocalizeSystem::OnStop);
		pComponentTypes->RegisterReferencesCallback<Localize>(LocalizeSystem::GetReferences);
	}

	void LocalizeModule::PostInitialize()
	{
	}

	void LocalizeModule::Update()
	{
	}

	void LocalizeModule::Cleanup()
	{
	}

	void LocalizeModule::LoadSettings(ModuleSettings& settings)
	{
	}
}

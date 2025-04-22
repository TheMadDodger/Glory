#include "LocalizeModule.h"
#include "StringTable.h"
#include "Localize.h"
#include "LocalizeSystem.h"

#include <Engine.h>
#include <AssetManager.h>
#include <SceneManager.h>

#include <EntityRegistry.h>
#include <BinaryStream.h>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(LocalizeModule);

	LocalizeModule::LocalizeModule() : m_DefaultLanguage("English"), m_CurrentLanguage(m_DefaultLanguage)
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
		if (std::find(m_LoadedTableIDs.begin(), m_LoadedTableIDs.end(), tableID) != m_LoadedTableIDs.end()) return;

		Resource* pResource = m_pEngine->GetAssetManager().FindResource(tableID);
		if (!pResource) return;

		StringTable* pTable = static_cast<StringTable*>(pResource);
		m_LoadedTableIDs.emplace_back(tableID);
		LoadedTable& table = m_LoadedTables.emplace_back(pTable->Name());

		for (auto iter = pTable->Begin(); iter != pTable->End(); ++iter)
		{
			table.m_Strings.emplace(iter->first, iter->second);
		}
	}

	void LocalizeModule::UnloadStringTable(UUID tableID)
	{
		auto iter = std::find(m_LoadedTableIDs.begin(), m_LoadedTableIDs.end(), tableID);
		if (iter == m_LoadedTableIDs.end()) return;

		const size_t index = iter - m_LoadedTableIDs.begin();
		m_LoadedTables.erase(m_LoadedTables.begin() + index);
		m_LoadedTableIDs.erase(iter);
	}

	void LocalizeModule::Clear()
	{
		m_LoadedTables.clear();
		m_LoadedTableIDs.clear();
	}

	bool LocalizeModule::FindString(const std::string_view tableName, const std::string_view term, std::string& out)
	{
		auto iter = std::find_if(m_LoadedTables.begin(), m_LoadedTables.end(),
			[&tableName](const LoadedTable& table) { return table.m_Name == tableName; });
		if (iter == m_LoadedTables.end()) return false;
		const LoadedTable& table = *iter;
		auto termIter = table.m_Strings.find(term);
		if (termIter == table.m_Strings.end()) return false;
		out = termIter->second;
		return true;
	}

	void LocalizeModule::SetLanguages(std::string&& defaultLanguage, std::vector<std::string>&& supportedLanguages)
	{
		m_DefaultLanguage = std::move(defaultLanguage);
		m_CurrentLanguage = m_DefaultLanguage;
		m_SupportedLanguages = std::move(supportedLanguages);
	}

	void LocalizeModule::Initialize()
	{
		Reflect::SetReflectInstance(&m_pEngine->Reflection());
		m_pEngine->GetResourceTypes().RegisterResource<StringTable>("");

		Reflect::RegisterType<StringTableRef>();
		Reflect::RegisterType<StringTableLoader>();
		Reflect::RegisterType<Localize>();

		Utils::ECS::ComponentTypes* pComponentTypes = m_pEngine->GetSceneManager()->ComponentTypesInstance();
		pComponentTypes->RegisterComponent<StringTableLoader>();
		pComponentTypes->RegisterComponent<Localize>();

		pComponentTypes->RegisterInvokaction<StringTableLoader>(Utils::ECS::InvocationType::OnValidate, StringTableLoaderSystem::OnValidate);
		pComponentTypes->RegisterInvokaction<StringTableLoader>(Utils::ECS::InvocationType::OnRemove, StringTableLoaderSystem::OnStop);
		pComponentTypes->RegisterInvokaction<StringTableLoader>(Utils::ECS::InvocationType::Stop, StringTableLoaderSystem::OnStop);
		pComponentTypes->RegisterReferencesCallback<StringTableLoader>(StringTableLoaderSystem::GetReferences);
		pComponentTypes->RegisterInvokaction<Localize>(Utils::ECS::InvocationType::OnValidate, LocalizeSystem::OnValidate);
		pComponentTypes->RegisterInvokaction<Localize>(Utils::ECS::InvocationType::Start, LocalizeSystem::OnStart);
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

	void LocalizeModule::OnProcessData()
	{
		if (!m_pEngine->HasData("Languages")) return;
		std::vector<char>& data = m_pEngine->GetData("Assemblies");
		BinaryMemoryStream memoryStream{ data };
		BinaryStream& stream = memoryStream;
		stream.Read(m_DefaultLanguage);
		stream.Read(m_SupportedLanguages);
		m_CurrentLanguage = m_DefaultLanguage;
	}
}

#include "LocalizeModule.h"
#include "StringTable.h"
#include "StringsOverrideTable.h"
#include "Localize.h"
#include "LocalizeSystem.h"

#include <Engine.h>
#include <Console.h>
#include <AssetManager.h>
#include <SceneManager.h>
#include <GScene.h>

#include <EntityRegistry.h>
#include <BinaryStream.h>
#include <AssetArchive.h>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(LocalizeModule);

	LocalizeModule::LocalizeModule() : m_DefaultLanguage("English"), m_CurrentLanguage(m_DefaultLanguage), m_LocalePath("./Data/Locale")
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

		/* Load override tables if we're not on the default language */
		if (m_CurrentLanguage == m_DefaultLanguage) return;
		for (const LocaleData& localeData : m_LocaleData)
		{
			if (localeData.m_Language != m_CurrentLanguage) continue;
			if (localeData.m_BaseTableID != tableID) continue;
			auto baseTableIter = std::find(m_LoadedTableIDs.begin(), m_LoadedTableIDs.end(), localeData.m_BaseTableID);
			if (baseTableIter == m_LoadedTableIDs.end()) continue;
			Resource* pResource = m_pEngine->GetAssetManager().FindResource(localeData.m_OverrideTableID);
			if (!pResource)
			{
				/* Must load it in */
				std::filesystem::path path = m_LocalePath;
				path.append(std::to_string(localeData.m_OverrideTableID)).replace_extension(".gcl");
				if (!std::filesystem::exists(path)) continue;
				BinaryFileStream stream{ path, true, false };
				AssetArchive archive{ &stream };
				archive.Deserialize(m_pEngine);
				if (archive.Size() != 1) continue;
				pResource = archive.Get(m_pEngine, 0);
				continue;
			}
			StringsOverrideTable* pOverrideTable = static_cast<StringsOverrideTable*>(pResource);
			LoadStringOverrideTable(pOverrideTable);
		}
	}

	void LocalizeModule::LoadStringOverrideTable(StringsOverrideTable* pTable)
	{
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(pTable->BaseTableID());
		if (!pResource) return;

		StringTable* pBaseTable = static_cast<StringTable*>(pResource);
		m_LoadedOverrideTableIDs.emplace_back(pTable->GetUUID());
		LoadedTable& table = m_LoadedOverrideTables.emplace_back(pBaseTable->Name());

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

		std::vector<UUID> toUnloadOverrideTables;
		for (size_t i = m_LoadedOverrideTables.size(); i > 0; --i)
		{
			const LoadedTable& table = m_LoadedOverrideTables[i - 1];
			if (m_LoadedTables[index].m_Name != table.m_Name) continue;
			UnloadStringOverrideTable(m_LoadedOverrideTableIDs[i - 1]);
		}

		m_LoadedTables.erase(m_LoadedTables.begin() + index);
		m_LoadedTableIDs.erase(iter);
	}

	void LocalizeModule::UnloadStringOverrideTable(UUID overrideTableID)
	{
		auto iter = std::find(m_LoadedOverrideTableIDs.begin(), m_LoadedOverrideTableIDs.end(), overrideTableID);
		if (iter == m_LoadedOverrideTableIDs.end()) return;

		const size_t index = iter - m_LoadedOverrideTableIDs.begin();
		m_LoadedOverrideTables.erase(m_LoadedOverrideTables.begin() + index);
		m_LoadedOverrideTableIDs.erase(iter);
	}

	void LocalizeModule::Clear()
	{
		m_LoadedTables.clear();
		m_LoadedTableIDs.clear();
		m_LoadedOverrideTables.clear();
		m_LoadedOverrideTableIDs.clear();
		m_LocaleData.clear();
	}

	bool LocalizeModule::FindString(const std::string_view tableName, const std::string_view term, std::string& out)
	{
		auto overrideIter = std::find_if(m_LoadedOverrideTables.begin(), m_LoadedOverrideTables.end(),
			[&tableName](const LoadedTable& table) { return table.m_Name == tableName; });
		if (overrideIter != m_LoadedOverrideTables.end())
		{
			const LoadedTable& overrideTable = *overrideIter;
			auto overrideTermIter = overrideTable.m_Strings.find(term);
			if (overrideTermIter != overrideTable.m_Strings.end())
			{
				out = overrideTermIter->second;
				return true;
			}
		}

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

	void LocalizeModule::SetLocaleDatas(std::vector<LocaleData>&& localeDatas)
	{
		m_LocaleData = std::move(localeDatas);
	}

	void LocalizeModule::SetLanguage(std::string_view language)
	{
		if (m_CurrentLanguage == language) return;
		if (language == m_DefaultLanguage)
		{
			m_CurrentLanguage = m_DefaultLanguage;
			m_LoadedOverrideTables.clear();
			RefreshText();
			return;
		}

		auto iter = std::find(m_SupportedLanguages.begin(), m_SupportedLanguages.end(), language);
		if (iter == m_SupportedLanguages.end())
			return;
		m_CurrentLanguage = *iter;

		m_LoadedOverrideTables.clear();

		for (const LocaleData& localeData : m_LocaleData)
		{
			if (localeData.m_Language != language) continue;
			auto baseTableIter = std::find(m_LoadedTableIDs.begin(), m_LoadedTableIDs.end(), localeData.m_BaseTableID);
			if (baseTableIter == m_LoadedTableIDs.end()) continue;
			Resource* pResource = m_pEngine->GetAssetManager().FindResource(localeData.m_OverrideTableID);
			if (!pResource)
			{
				/* Must load it in */
				std::filesystem::path path = m_LocalePath;
				path.append(std::to_string(localeData.m_OverrideTableID)).replace_extension(".gcl");
				if (!std::filesystem::exists(path)) continue;
				BinaryFileStream stream{ path, true, false };
				AssetArchive archive{ &stream };
				archive.Deserialize(m_pEngine);
				if (archive.Size() != 1) continue;
				pResource = archive.Get(m_pEngine, 0);
			}
			StringsOverrideTable* pOverrideTable = static_cast<StringsOverrideTable*>(pResource);
			LoadStringOverrideTable(pOverrideTable);
		}
		RefreshText();
	}

	size_t LocalizeModule::LanguageCount() const
	{
		return m_SupportedLanguages.size() + 1;
	}

	std::string_view LocalizeModule::GetLanguage(size_t index) const
	{
		return index == 0 ? m_DefaultLanguage : m_SupportedLanguages[index - 1];
	}

	void LocalizeModule::Initialize()
	{
		Reflect::SetReflectInstance(&m_pEngine->Reflection());
		m_pEngine->GetResourceTypes().RegisterResource<StringTable>("");
		m_pEngine->GetResourceTypes().RegisterResource<StringsOverrideTable>("");

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

		m_pEngine->GetConsole().RegisterCommand(new ConsoleCommand1<std::string>("setLanguage", [this](const std::string& language) {
			SetLanguage(language);
			return true;
		}));
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
		if (m_pEngine->HasData("Languages"))
		{
			std::vector<char>& data = m_pEngine->GetData("Languages");
			BinaryMemoryStream memoryStream{ data };
			BinaryStream& stream = memoryStream;
			stream.Read(m_DefaultLanguage);
			stream.Read(m_SupportedLanguages);
			m_CurrentLanguage = m_DefaultLanguage;
		}
		if (m_pEngine->HasData("Locale"))
		{
			std::filesystem::path localePath = m_pEngine->DataPath("Locale");
			localePath = localePath.parent_path();
			localePath.append("Locale");
			m_LocalePath = localePath.string();

			std::vector<char>& data = m_pEngine->GetData("Locale");
			BinaryMemoryStream memoryStream{ data };
			BinaryStream& stream = memoryStream;

			size_t numLocaleDatas;
			stream.Read(numLocaleDatas);
			m_LocaleData.resize(numLocaleDatas);

			for (LocaleData& localeData : m_LocaleData)
			{
				stream.Read(localeData.m_BaseTableID).Read(localeData.m_OverrideTableID).Read(localeData.m_Language);
			}
		}
	}

	void LocalizeModule::RefreshText()
	{
		/* Trigger start on localize components to refresh text */
		for (size_t i = 0; i < m_pEngine->GetSceneManager()->OpenScenesCount(); ++i)
		{
			GScene* pScene = m_pEngine->GetSceneManager()->GetOpenScene(i);
			pScene->GetRegistry().InvokeAll<Localize>(Utils::ECS::InvocationType::Start);
		}
		if (OnLanguageChanged) OnLanguageChanged();
	}
}

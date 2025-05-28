#include "LocalizeCSAPI.h"

#include <EntityCSAPI.h>

#include <cstdint>
#include <GloryMonoScipting.h>
#include <Debug.h>
#include <Engine.h>
#include <GScene.h>
#include <AssetManager.h>
#include <Localize.h>
#include <LocalizeModule.h>
#include <Components.h>

namespace Glory
{
	Engine* Localize_EngineInstance;
#define LOCALIZE_MODULE Localize_EngineInstance->GetOptionalModule<LocalizeModule>()

	template<typename T>
	static T& GetComponent(UUID sceneID, UUID objectID, uint64_t componentID)
	{
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		Utils::ECS::EntityView* pEntityView = entity.GetEntityView();
		return pScene->GetRegistry().GetComponent<T>(entity.GetEntityID());
	}

#pragma region Locale

	MonoString* Locale_GetCurrentLanguage()
	{
		const std::string_view language = LOCALIZE_MODULE->CurrentLanguage();
		return mono_string_new(mono_domain_get(), language.data());
	}

	void Locale_SetCurrentLanguage(MonoString* language)
	{
		const std::string_view languageStr = mono_string_to_utf8(language);
		LOCALIZE_MODULE->SetLanguage(languageStr);
	}

	uint32_t Locale_GetLanguageCount()
	{
		return (uint32_t)LOCALIZE_MODULE->LanguageCount();
	}

	MonoString* Locale_GetLanguage(uint32_t index)
	{
		const std::string_view languageStr = LOCALIZE_MODULE->GetLanguage(size_t(index));
		return mono_string_new(mono_domain_get(), languageStr.data());
	}

	MonoString* Locale_Translate(MonoString* term)
	{
		const std::string_view termStr = mono_string_to_utf8(term);
		const size_t firstDot = termStr.find('.');
		if (firstDot == std::string::npos) return term;
		const std::string_view tableName = termStr.substr(0, firstDot);
		const std::string_view termName = termStr.substr(firstDot + 1);
		std::string translation;
		return LOCALIZE_MODULE->FindString(tableName, termName, translation) ?
			mono_string_new(mono_domain_get(), translation.data()) : term;
	}

#pragma endregion

#pragma region Localize component

	MonoString* Localize_GetTerm(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		Localize& localize = GetComponent<Localize>(sceneID, objectID, componentID);
		return localize.m_Term.empty() ? nullptr : mono_string_new(mono_domain_get(), localize.m_Term.data());
	}

	void Localize_SetTerm(uint64_t sceneID, uint64_t objectID, uint64_t componentID, MonoString* term)
	{
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		Localize& localize = entity.GetComponent<Localize>();
		localize.m_Term = term ? mono_string_to_utf8(term) : "";

		/* Refresh text */
		if (!entity.HasComponent<TextComponent>()) return;
		TextComponent& text = entity.GetComponent<TextComponent>();
		const std::string_view fullTerm = localize.m_Term;
		const size_t firstDot = fullTerm.find('.');
		if (firstDot == std::string::npos) return;
		const std::string_view tableName = fullTerm.substr(0, firstDot);
		const std::string_view termName = fullTerm.substr(firstDot + 1);
		text.m_Dirty |= LOCALIZE_MODULE->FindString(tableName, termName, text.m_Text);
	}

#pragma endregion

#pragma region StringTableLoader

	uint64_t StringTableLoader_GetTableToLoadID(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		StringTableLoader& tableLoader = GetComponent<StringTableLoader>(sceneID, objectID, componentID);
		return tableLoader.m_StringTable.AssetUUID();
	}

	void StringTableLoader_SetTableToLoadID(uint64_t sceneID, uint64_t objectID, uint64_t componentID, uint64_t tableID)
	{
		StringTableLoader& tableLoader = GetComponent<StringTableLoader>(sceneID, objectID, componentID);
		tableLoader.m_StringTable.SetUUID(tableID);
	}

	bool StringTableLoader_GetKeepLoaded(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		StringTableLoader& tableLoader = GetComponent<StringTableLoader>(sceneID, objectID, componentID);
		return tableLoader.m_KeepLoaded;
	}

	void StringTableLoader_SetKeepLoaded(uint64_t sceneID, uint64_t objectID, uint64_t componentID, bool value)
	{
		StringTableLoader& tableLoader = GetComponent<StringTableLoader>(sceneID, objectID, componentID);
		tableLoader.m_KeepLoaded = value;
	}

#pragma endregion

#pragma region String Table

	MonoArray* StringTable_FindKeys(uint64_t tableID, MonoString* path)
	{
		const std::string pathStr = mono_string_to_utf8(path);
		Resource* pTableResource = Localize_EngineInstance->GetAssetManager().FindResource(tableID);
		if (!pTableResource) return nullptr;
		StringTable* pTable = static_cast<StringTable*>(pTableResource);
		std::vector<std::string>* keys = pTable->FindKeys(pathStr);
		if (keys == nullptr) return nullptr;
		
		MonoClass* pStringClass = mono_get_string_class();
		MonoArray* pArray = mono_array_new(mono_domain_get(), pStringClass, keys->size());
		for (size_t i = 0; i < keys->size(); ++i)
		{
			MonoString* pMonoString = mono_string_new(mono_domain_get(), (*keys)[i].data());
			mono_array_setref(pArray, i, pMonoString);
		}
		return pArray;
	}

	uint32_t StringTable_FindKeysNoAlloc(uint64_t tableID, MonoString* path, MonoArray* pArray, uint32_t maxKeys)
	{
		const std::string pathStr = mono_string_to_utf8(path);
		Resource* pTableResource = Localize_EngineInstance->GetAssetManager().FindResource(tableID);
		if (!pTableResource) return 0;
		StringTable* pTable = static_cast<StringTable*>(pTableResource);
		std::vector<std::string>* keys = pTable->FindKeys(pathStr);
		if (keys == nullptr) return 0;

		const uint32_t keyCount = size_t(std::fmin(keys->size(), maxKeys));
		for (size_t i = 0; i < keyCount; ++i)
		{
			MonoString* pMonoString = mono_string_new(mono_domain_get(), (*keys)[i].data());
			mono_array_setref(pArray, i, pMonoString);
		}
		return keyCount;
	}

	MonoArray* StringTable_FindKeysRecursive(uint64_t tableID, MonoString* path)
	{
		const std::string pathStr = mono_string_to_utf8(path);
		Resource* pTableResource = Localize_EngineInstance->GetAssetManager().FindResource(tableID);
		if (!pTableResource) return nullptr;
		StringTable* pTable = static_cast<StringTable*>(pTableResource);
		std::vector<std::string> keys;
		pTable->FindKeysRecursively(pathStr, keys);
		if (keys.empty()) return nullptr;

		MonoClass* pStringClass = mono_get_string_class();
		MonoArray* pArray = mono_array_new(mono_domain_get(), pStringClass, keys.size());
		for (size_t i = 0; i < keys.size(); ++i)
		{
			MonoString* pMonoString = mono_string_new(mono_domain_get(), keys[i].data());
			mono_array_setref(pArray, i, pMonoString);
		}
		return pArray;
	}

	uint32_t StringTable_FindKeysRecursiveNoAlloc(uint64_t tableID, MonoString* path, MonoArray* pArray, uint32_t maxKeys)
	{
		const std::string pathStr = mono_string_to_utf8(path);
		Resource* pTableResource = Localize_EngineInstance->GetAssetManager().FindResource(tableID);
		if (!pTableResource) return 0;
		StringTable* pTable = static_cast<StringTable*>(pTableResource);
		std::vector<std::string> keys;
		pTable->FindKeysRecursively(pathStr, keys);
		if (keys.empty()) return 0;

		const uint32_t keyCount = size_t(std::fmin(keys.size(), maxKeys));
		for (size_t i = 0; i < keyCount; ++i)
		{
			MonoString* pMonoString = mono_string_new(mono_domain_get(), keys[i].data());
			mono_array_setref(pArray, i, pMonoString);
		}
		return keyCount;
	}

	MonoArray* StringTable_FindSubgroups(uint64_t tableID, MonoString* path)
	{
		const std::string pathStr = mono_string_to_utf8(path);
		Resource* pTableResource = Localize_EngineInstance->GetAssetManager().FindResource(tableID);
		if (!pTableResource) return nullptr;
		StringTable* pTable = static_cast<StringTable*>(pTableResource);
		std::vector<std::string_view> groups;
		pTable->FindSubgroups(pathStr, groups);
		if (groups.empty()) return nullptr;

		MonoClass* pStringClass = mono_get_string_class();
		MonoArray* pArray = mono_array_new(mono_domain_get(), pStringClass, groups.size());
		for (size_t i = 0; i < groups.size(); ++i)
		{
			MonoString* pMonoString = mono_string_new(mono_domain_get(), groups[i].data());
			mono_array_setref(pArray, i, pMonoString);
		}
		return pArray;
	}

	uint32_t StringTable_FindSubgroupsNoAlloc(uint64_t tableID, MonoString* path, MonoArray* pArray, uint32_t maxGroups)
	{
		const std::string pathStr = mono_string_to_utf8(path);
		Resource* pTableResource = Localize_EngineInstance->GetAssetManager().FindResource(tableID);
		if (!pTableResource) return 0;
		StringTable* pTable = static_cast<StringTable*>(pTableResource);
		std::vector<std::string_view> groups;
		pTable->FindSubgroups(pathStr, groups);
		if (groups.empty()) return 0;

		const uint32_t keyCount = size_t(std::fmin(groups.size(), maxGroups));
		for (size_t i = 0; i < groups.size(); ++i)
		{
			MonoString* pMonoString = mono_string_new(mono_domain_get(), groups[i].data());
			mono_array_setref(pArray, i, pMonoString);
		}
		return keyCount;
	}

#pragma endregion

#pragma region Binding

	void LocalizeCSAPI::AddInternalCalls(std::vector<InternalCall>& internalCalls)
	{
        /* Locale */
        BIND("GloryEngine.Localize.Locale::Locale_GetCurrentLanguage", Locale_GetCurrentLanguage);
        BIND("GloryEngine.Localize.Locale::Locale_SetCurrentLanguage", Locale_SetCurrentLanguage);
        BIND("GloryEngine.Localize.Locale::Locale_GetLanguageCount", Locale_GetLanguageCount);
        BIND("GloryEngine.Localize.Locale::Locale_GetLanguage", Locale_GetLanguage);
        BIND("GloryEngine.Localize.Locale::Locale_Translate", Locale_Translate);

		/* Localize component */
        BIND("GloryEngine.Localize.Localize::Localize_GetTerm", Localize_GetTerm);
        BIND("GloryEngine.Localize.Localize::Localize_SetTerm", Localize_SetTerm);

		/* StringTableLoader component */
        BIND("GloryEngine.Localize.StringTableLoader::StringTableLoader_GetTableToLoadID", StringTableLoader_GetTableToLoadID);
        BIND("GloryEngine.Localize.StringTableLoader::StringTableLoader_SetTableToLoadID", StringTableLoader_SetTableToLoadID);
        BIND("GloryEngine.Localize.StringTableLoader::StringTableLoader_GetKeepLoaded", StringTableLoader_GetKeepLoaded);
        BIND("GloryEngine.Localize.StringTableLoader::StringTableLoader_SetKeepLoaded", StringTableLoader_SetKeepLoaded);

		/* StringTable */
        BIND("GloryEngine.Localize.StringTable::StringTable_FindKeys", StringTable_FindKeys);
        BIND("GloryEngine.Localize.StringTable::StringTable_FindKeysNoAlloc", StringTable_FindKeysNoAlloc);
        BIND("GloryEngine.Localize.StringTable::StringTable_FindSubgroups", StringTable_FindSubgroups);
        BIND("GloryEngine.Localize.StringTable::StringTable_FindSubgroupsNoAlloc", StringTable_FindSubgroupsNoAlloc);
        BIND("GloryEngine.Localize.StringTable::StringTable_FindKeysRecursive", StringTable_FindKeysRecursive);
        BIND("GloryEngine.Localize.StringTable::StringTable_FindKeysRecursiveNoAlloc", StringTable_FindKeysRecursiveNoAlloc);
	}

	void LocalizeCSAPI::SetEngine(Engine* pEngine)
	{
		Localize_EngineInstance = pEngine;
	}

#pragma endregion

}
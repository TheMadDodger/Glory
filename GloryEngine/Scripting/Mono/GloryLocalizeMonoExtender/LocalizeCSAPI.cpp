#include "LocalizeCSAPI.h"

#include <cstdint>
#include <GloryMonoScipting.h>
#include <Debug.h>
#include <Engine.h>
#include <AssetManager.h>
#include <Localize.h>
#include <LocalizeModule.h>

namespace Glory
{
	Engine* Localize_EngineInstance;
#define LOCALIZE_MODULE Localize_EngineInstance->GetOptionalModule<LocalizeModule>()

#pragma region Localize

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

#pragma region Binding

	void LocalizeCSAPI::AddInternalCalls(std::vector<InternalCall>& internalCalls)
	{
        /* Localize */
        BIND("GloryEngine.Localize.Locale::Locale_GetCurrentLanguage", Locale_GetCurrentLanguage);
        BIND("GloryEngine.Localize.Locale::Locale_SetCurrentLanguage", Locale_SetCurrentLanguage);
        BIND("GloryEngine.Localize.Locale::Locale_GetLanguageCount", Locale_GetLanguageCount);
        BIND("GloryEngine.Localize.Locale::Locale_GetLanguage", Locale_GetLanguage);
        BIND("GloryEngine.Localize.Locale::Locale_Translate", Locale_Translate);
	}

	void LocalizeCSAPI::SetEngine(Engine* pEngine)
	{
		Localize_EngineInstance = pEngine;
	}

#pragma endregion

}
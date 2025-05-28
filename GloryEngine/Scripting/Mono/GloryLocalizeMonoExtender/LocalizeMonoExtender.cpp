#include "LocalizeMonoExtender.h"
#include "LocalizeCSAPI.h"

#include <MonoManager.h>
#include <AssemblyDomain.h>
#include <CoreLibManager.h>
#include <LocalizeModule.h>
#include <GloryMonoScipting.h>
#include <ScriptingExtender.h>
#include <Engine.h>
#include <Assembly.h>
#include <Debug.h>
#include <StringsOverrideTable.h>

namespace Glory
{
	LocalizeMonoExtender::LocalizeMonoExtender(const char* path) : m_pLibManager(new LocalizeLibManager()), m_Path(path)
	{
	}

	LocalizeMonoExtender::~LocalizeMonoExtender()
	{
		delete m_pLibManager;
		m_pLibManager = nullptr;
	}

	void LocalizeMonoExtender::GetInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		LocalizeCSAPI::AddInternalCalls(internalCalls);
	}

	void LocalizeMonoExtender::GetLibs(ScriptingExtender* pScriptingExtender)
	{
		pScriptingExtender->AddInternalLib(m_Path, "GloryEngine.Localize.dll", m_pLibManager);
	}

	bool OnLoadExtra(const char* path, Module* pModule, Module* pRequiredModule)
	{
		GloryMonoScipting* pScripting = (GloryMonoScipting*)pRequiredModule;
		IScriptExtender* pScriptExtender = new LocalizeMonoExtender(path);
		pScripting->GetScriptingExtender()->RegisterExtender(pScriptExtender);
		return true;
	}

	void LocalizeLibManager::CollectTypes(Engine*, Assembly*)
	{
		AddMonoType("GloryEngine.Localize.StringTable", SerializedType::ST_Asset, ResourceTypes::GetHash<StringTable>());
		AddMonoType("GloryEngine.Localize.StringsOverrideTable", SerializedType::ST_Asset, ResourceTypes::GetHash<StringsOverrideTable>());
	}

	void LocalizeLibManager::Initialize(Engine* pEngine, Assembly* pAssembly)
	{
		m_pAssembly = pAssembly;

		LocalizeCSAPI::SetEngine(pEngine);

		MonoManager* pMonoManager = MonoManager::Instance();
		AssemblyClass* pLocaleClass = m_pAssembly->GetClass("GloryEngine.Localize", "Locale");

		LocalizeModule* pLocalizeModule = pEngine->GetOptionalModule<LocalizeModule>();
		MonoMethod* pLanguageChangedMethod = pLocaleClass->GetMethod(".::LanguageChanged");

		AssemblyDomain* pDomain = pAssembly->GetDomain();

		pLocalizeModule->LanguageChanged =
		[pDomain, pLanguageChangedMethod](std::string_view language) {
			MonoString* pMonoString = mono_string_new(pDomain->GetMonoDomain(), language.data());
			void* args[] = {
				pMonoString,
			};
			pDomain->InvokeMethod(pLanguageChangedMethod, nullptr, args);
		};
	}

	void LocalizeLibManager::Cleanup(Engine*)
	{
	}

	void LocalizeLibManager::Reset(Engine*)
	{
		AssemblyClass* pFSDMManagerClass = m_pAssembly->GetClass("GloryEngine.Localize", "Locale");
		MonoMethod* pReset = pFSDMManagerClass->GetMethod(".::Reset");
		MonoObject* pExcept;
		MonoObject* pReturn = mono_runtime_invoke(pReset, nullptr, nullptr, &pExcept);
		if (pExcept)
			mono_print_unhandled_exception(pExcept);
	}
}

#include "GloryMonoScipting.h"
#include "CoreCSAPI.h"
#include "MathCSAPI.h"
#include "MonoLibManager.h"
#include "MonoManager.h"
#include <Game.h>

namespace Glory
{
	GloryMonoScipting::GloryMonoScipting() : ScriptingModuleTemplate("csharp")
	{
	}

	GloryMonoScipting::~GloryMonoScipting()
	{
		
	}

	void GloryMonoScipting::Initialize()
	{
		m_pEngine->GetScriptingExtender()->RegisterExtender(this, this);

		//if (Game::GetGame().GetApplicationType() == ApplicationType::AT_Editor) return;
		MonoManager::Initialize("./Modules/GloryMonoScripting/Dependencies");
	}
	
	void GloryMonoScipting::SetMonoDirs(const std::string& assemblyDir, const std::string& configDir)
	{
		mono_set_dirs(assemblyDir.c_str(), configDir.c_str());
	}

	void GloryMonoScipting::PostInitialize()
	{
		
	}

	void GloryMonoScipting::InitializeScripting()
	{
		//AssemblyBinding* pAssembly = MonoLibManager::GetAssembly("GloryEngine.Core.dll");
		//MonoClass* pMainClass = mono_class_from_name(pAssembly->GetMonoImage(), "Glory", "GloryCSMain");
		//mono_bool result = mono_class_init(pMainClass);
		//MonoObject* pMonoObject1 = mono_object_new(mono_domain_get(), pMainClass);
		//
		//MonoMethodDesc* pMainFuncDesc = mono_method_desc_new(".GloryMain:main()", false);
		//MonoMethod* pMainFunc = mono_method_desc_search_in_class(pMainFuncDesc, pMainClass);
		//// Exception object
		//MonoObject* pExObject = nullptr;
		//MonoObject* pMonoObject = mono_runtime_invoke(pMainFunc, nullptr, nullptr, &pExObject);
		//uint32_t monoObjectGCHandle = mono_gchandle_new(pMonoObject, false);
		//mono_method_desc_free(pMainFuncDesc);
	}

	void GloryMonoScipting::Cleanup()
	{
		MonoManager::Cleanup();
	}

	void GloryMonoScipting::LoadLib(const ScriptingLib& library)
	{
		MonoManager::LoadLib(library);
	}

	void GloryMonoScipting::Bind(const InternalCall& internalCall)
	{
		const std::string& scriptMethod = internalCall.ScriptMethod();
		mono_add_internal_call(scriptMethod.c_str(), internalCall.RedirectMethod());
	}

	std::string GloryMonoScipting::Language()
	{
		return ScriptingLanguage();
	}

	void GloryMonoScipting::GetInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		CoreCSAPI::AddInternalCalls(internalCalls);
		MathCSAPI::AddInternalCalls(internalCalls);
	}

	void GloryMonoScipting::GetLibs(ScriptingExtender* pScriptingExtender)
	{
		pScriptingExtender->AddInternalLib("GloryEngine.Core.dll");
	}
}

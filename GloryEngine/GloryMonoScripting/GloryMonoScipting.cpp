#include "GloryMonoScipting.h"
#include "DebugBinder.h"
#include "MathBinder.h"
#include <mono/metadata/assembly.h>
#include <Game.h>
#include <mono/metadata/debug-helpers.h>
#include <ScriptBinding.h>

namespace Glory
{
	GloryMonoScipting::GloryMonoScipting() : ScriptingModuleTemplate("C#"), m_pDomain(nullptr)
	{
	}

	GloryMonoScipting::~GloryMonoScipting()
	{
		m_Assemblies.clear();
		m_pDomain = nullptr;
	}

	void GloryMonoScipting::Initialize()
	{
		//if (Game::GetGame().GetApplicationType() == ApplicationType::AT_Editor) return;
		mono_set_dirs(".", "");
		InitializeDomainAndMainAssembly();
	}
	
	void GloryMonoScipting::SetMonoDirs(const std::string& assemblyDir, const std::string& configDir)
	{
		mono_set_dirs(assemblyDir.c_str(), configDir.c_str());
	}

	void GloryMonoScipting::InitializeDomainAndMainAssembly()
	{
		m_pDomain = mono_jit_init_version("GloryMain", "v4.0.30319");
		//m_Assemblies.push_back(AssemblyBinding(m_pDomain, "./GloryCS.dll"));
		//m_Assemblies.push_back(AssemblyBinding(m_pDomain, "C:/Users/Sparta/Documents/Glory/GloryEngine/GloryCS/Debug/Win32/GloryCS.dll"));
		m_Assemblies.push_back(AssemblyBinding(m_pDomain, "./GloryMonoScriptingCS.dll"));
	}

	class RandomWord
	{
	public:
		static MonoString* GetWord()
		{
			return mono_string_new(mono_domain_get(), "Hello World 42069");
		}
	};

	void GloryMonoScipting::PostInitialize()
	{
		for (size_t i = 0; i < m_Assemblies.size(); i++)
		{
			m_Assemblies[i].Initialize();
		}

		mono_add_internal_call("GloryEngine.Debug::GetRandomWord()", &RandomWord::GetWord);

		MonoClass* pMainClass = mono_class_from_name(m_Assemblies[0].GetMonoImage(), "Glory", "GloryCSMain");
		MonoMethodDesc* pMainFuncDesc = mono_method_desc_new(".GloryMain:main()", false);
		MonoMethod* pMainFunc = mono_method_desc_search_in_class(pMainFuncDesc, pMainClass);
		// Exception object
		MonoObject* pExObject = nullptr;
		MonoObject* pMonoObject = mono_runtime_invoke(pMainFunc, nullptr, nullptr, &pExObject);
		uint32_t monoObjectGCHandle = mono_gchandle_new(pMonoObject, false);
		mono_method_desc_free(pMainFuncDesc);
	}

	void GloryMonoScipting::Cleanup()
	{
		for (size_t i = 0; i < m_Assemblies.size(); i++)
		{
			m_Assemblies[i].Destroy();
		}

		if (m_pDomain) mono_jit_cleanup(m_pDomain);
	}

	void GloryMonoScipting::GetScriptBindings(std::vector<ScriptBinding>& bindings)
	{
		DebugBinder::CreateBindings(bindings);
		MathBinder::CreateBindings(bindings);
	}

	void GloryMonoScipting::Bind(const ScriptBinding& binding)
	{
		const std::string& scriptMethod = binding.ScriptMethod();
		mono_add_internal_call(scriptMethod.c_str(), binding.RedirectMethod());
	}
}

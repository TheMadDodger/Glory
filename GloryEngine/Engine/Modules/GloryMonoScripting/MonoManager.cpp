#include "MonoManager.h"
#include "MonoSceneManager.h"
#include "GloryMonoScipting.h"
#include "MonoScriptObjectManager.h"
#include "CoreLibManager.h"
#include "ScriptingMethodsHelper.h"
#include "AssemblyDomain.h"

#include <mono/jit/jit.h>
#include <mono/utils/mono-counters.h>
#include <mono/utils/mono-logger.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/object.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/profiler.h>


mono_bool OnStackWalk(MonoMethod* method, int32_t native_offset, int32_t il_offset, mono_bool managed, void* data)
{
    //auto result = (StackWalkDataResult*)data;
    //
    //if (method)
    //{
    //    auto mName = mono_method_get_name(method);
    //    auto mKlassNameSpace = mono_class_get_namespace(mono_method_get_class(method));
    //    auto mKlassName = mono_class_get_name(mono_method_get_class(method));
    //    result->Buffer.Append(mKlassNameSpace);
    //    result->Buffer.Append(TEXT("."));
    //    result->Buffer.Append(mKlassName);
    //    result->Buffer.Append(TEXT("::"));
    //    result->Buffer.Append(mName);
    //    result->Buffer.Append(TEXT("\n"));
    //}
    //else if (!managed)
    //{
    //    result->Buffer.Append(TEXT("<unmanaged>\n"));
    //}

    return 0;
}

namespace Glory
{
	struct GMonoProfiler
	{
		static constexpr unsigned long long MaxGCAllocations = 250000;

		/* Handle obtained from mono_profiler_create (). */
		MonoProfilerHandle Handle;

		/* Counts the number of calls observed. */
		unsigned long long NCalls;

		unsigned long long ActiveGCBytes;

		MonoManager* m_pMonoManager;
	};

	GMonoProfiler GProfiler;

	MonoManager* MonoManager::m_pInstance = nullptr;

	std::map<std::string, Debug::LogLevel> MONOTOLOGLEVEL = {
		{"error", Debug::LogLevel::Error},
		{"critical", Debug::LogLevel::FatalError},
		{"warning", Debug::LogLevel::Warning},
		{"message", Debug::LogLevel::Notice},
		{"info", Debug::LogLevel::Info},
		{"debug", Debug::LogLevel::Info},
	};

	void OnLogCallback(const char* logDomain, const char* logLevel, const char* message, mono_bool fatal, void* userData)
	{
		const Debug::LogLevel debugLogLevel = logLevel ? MONOTOLOGLEVEL.at(logLevel) : Debug::LogLevel::Info;

		std::stringstream str;
		str << "MONO: ";
		if (logDomain) str << "Domain: " << logDomain << ": ";
		if (logLevel) str << logLevel << ": ";
		if (message) str << message;
		if (fatal)
		{
			Glory::Debug::LogFatalError(str.str());
			return;
		}
		Glory::Debug::Log(str.str(), debugLogLevel);
	}

	void OnGCAllocation(MonoProfiler* pProfiler, MonoObject* obj)
	{
		// Get allocation info
		MonoClass* pMonoClass = mono_object_get_class(obj);
		const char* nameSpace = mono_class_get_namespace(pMonoClass);
		const char* name = mono_class_get_name(pMonoClass);
		const int32_t size = mono_class_instance_size(pMonoClass);

		std::stringstream log;
		log << "New GC allocation of type " << nameSpace << "." << name << " with size " << size << " bytes";
		//Glory::Debug::LogInfo(log.str());

		GMonoProfiler* pGProfiler = (GMonoProfiler*)pProfiler;
		pGProfiler->ActiveGCBytes += size;

		if (pGProfiler->ActiveGCBytes < GMonoProfiler::MaxGCAllocations) return;
		pGProfiler->m_pMonoManager->CollectGC();
		pGProfiler->ActiveGCBytes = 0;
	}

	void OnGCEvent(MonoProfiler* profiler, MonoProfilerGCEvent event, uint32_t generation, mono_bool is_serial)
	{
	}

	void OnPrintCallback(const char* string, mono_bool)
	{
		Debug::LogWarning(string);
	}

	void OnPrintErrorCallback(const char* string, mono_bool)
	{
		Debug::LogError(string);
	}

	void MonoManager::Initialize(const std::string& assemblyDir, const std::string& configDir)
	{
        /* Setup AOT */
		mono_jit_set_aot_mode(MonoAotMode::MONO_AOT_MODE_NONE);

        /* Setup mono dirs */
		mono_set_dirs(assemblyDir.c_str(), configDir.c_str());

        /* Set mono log level */
        const MonoLogLevel logLevel = m_pModule->Settings().EnumValue<MonoLogLevel>("MonoLogLevel");
        std::string logLevelString;
        if (!GloryReflect::Enum<MonoLogLevel>().ToString(logLevel, logLevelString))
            logLevelString = "error";
        mono_trace_set_level_string(logLevelString.c_str());

        /* Setup debugger */
        const std::string debugAgentIP = m_pModule->Settings().Value<std::string>("MonoDebuggingIP");
        const uint16_t debugAgentPort = m_pModule->Settings().Value<uint16_t>("MonoDebuggingPort");
        std::stringstream debuggerAgentStream;
        debuggerAgentStream << "--debugger-agent=transport=dt_socket,address=" << debugAgentIP << ":" << debugAgentPort << ",embedding=1,server=y,suspend=n,loglevel=10";

        const std::string debuggerAgentString = debuggerAgentStream.str();

		const char* options[] = {
		    "--soft-breakpoints",
            //"--debugger-agent=transport=dt_socket,address=127.0.0.1:55555"
            debuggerAgentString.c_str(),
		};
		mono_jit_parse_options(sizeof(options) / sizeof(char*), (char**)options);

		mono_debug_init(MONO_DEBUG_FORMAT_MONO);

        /* Setup log callbacks */
        mono_trace_set_log_handler(OnLogCallback, nullptr);
        mono_trace_set_print_handler(OnPrintCallback);
        mono_trace_set_printerr_handler(OnPrintErrorCallback);

		mono_config_parse(nullptr);

		GProfiler.m_pMonoManager = this;
		GProfiler.Handle = mono_profiler_create((MonoProfiler*)&GProfiler);
		mono_profiler_set_gc_allocation_callback(GProfiler.Handle, &OnGCAllocation);
		mono_profiler_set_gc_event_callback(GProfiler.Handle, &OnGCEvent);
		mono_profiler_enable_allocations();

		const char* monoVersion = "v4.0.30319";

		MonoDomain* pMonoDomain = mono_jit_init_version("GloryDomain", monoVersion);
		m_pRootDomain = new AssemblyDomain("root", pMonoDomain);
		m_Domains.emplace("root", m_pRootDomain);
		m_pActiveDomain = m_pRootDomain;
		//mono_debug_domain_create(m_pDomain);
		//mono_domain_set(m_pDomain, false);

		//mono_domain_set_config(m_pRootDomain, configDir.c_str(), configFilename.c_str());
		mono_thread_set_main(mono_thread_current());

		char* buildInfo = mono_get_runtime_build_info();
		std::stringstream buildInfoLog;
		buildInfoLog << "Mono Version: " << buildInfo;
		Debug::LogInfo(buildInfoLog.str());
		mono_free(buildInfo);
	}

	void MonoManager::Cleanup()
	{
		MonoSceneManager::Cleanup();

		m_pMethodsHelper->Cleanup();
		m_pCoreLibManager->Cleanup();

		for (auto& itor : m_Domains)
		{
			if (itor.first == "root") continue;
			UnloadDomain(itor.first, false);
		}

		CollectGC();

		if (mono_gc_pending_finalizers())
		{
			mono_gc_finalize_notify();
			while (mono_gc_pending_finalizers());
			{
				std::this_thread::sleep_for(std::chrono::nanoseconds(1));
			}
		}

		m_pRootDomain->Unload(true);
		mono_jit_cleanup(m_pRootDomain->GetMonoDomain());
		m_pRootDomain = nullptr;
		m_pActiveDomain = nullptr;
		m_Domains.clear();
	}

	void MonoManager::InitialLoad()
	{
		for (size_t i = 0; i < m_Libs.size(); ++i)
		{
			m_pActiveDomain->LoadLib(m_Libs[i]);
		}
		m_HadInitialLoad = true;
	}

	AssemblyDomain* MonoManager::GetDomain(const std::string& name)
	{
		auto itor = m_Domains.find(name);
		if (itor == m_Domains.end()) return nullptr;
		return itor->second;
	}

	void MonoManager::AddLib(const ScriptingLib& lib)
	{
		m_Libs.push_back(lib);

		if (!m_HadInitialLoad) return;
		m_pActiveDomain->LoadLib(lib);
	}

	//void MonoManager::Reload()
	//{
	//	// Create new domain
	//	//MonoDomain* pNewDomain = mono_domain_create_appdomain("GloryDomain", NULL);
	//	//mono_domain_set(pNewDomain, false);
	//	//mono_debug_domain_create(pNewDomain);

	//	// Unload domain
	//	//mono_debug_domain_unload(m_pDomain);
	//	//mono_domain_unload(m_pDomain);
	//	//mono_domain_finalize(m_pDomain, 2000);

	//	// Load assemblies into this domain
	//	//m_pRootDomain->ReloadAll();
	//}

	CoreLibManager* MonoManager::GetCoreLibManager() const
	{
		return m_pCoreLibManager;
	}

	ScriptingMethodsHelper* MonoManager::GetMethodsHelper() const
	{
		return m_pMethodsHelper;
	}

	AssemblyDomain* MonoManager::CreateDomain(const std::string& name)
	{
		auto itor = m_Domains.find(name);
		if (itor != m_Domains.end()) return itor->second;

		const auto pMonoDomain = mono_domain_create_appdomain((char*)name.data(), nullptr);
		mono_debug_domain_create(pMonoDomain);
		AssemblyDomain* pDomain = new AssemblyDomain(name, pMonoDomain);
		m_Domains.emplace(name, pDomain);
		return pDomain;
	}

	AssemblyDomain* MonoManager::ActiveDomain()
	{
		return m_pActiveDomain;
	}

	void MonoManager::UnloadDomain(const std::string& name, bool remove)
	{
		auto itor = m_Domains.find(name);
		if (itor == m_Domains.end()) return;

		AssemblyDomain* pDomain = itor->second;
		mono_debug_domain_unload(pDomain->GetMonoDomain());

		MonoObject* exception = nullptr;
		mono_domain_try_unload(pDomain->GetMonoDomain(), &exception);
		if (exception)
		{
			Debug::LogFatalError("An exception was thrown when trying to unload a domain");
			return;
		}
		delete pDomain;
		
		if (!remove) return;
		m_Domains.erase(itor);
	}

	void MonoManager::Reload()
	{
		UnloadDomain("GloryDomain");
		AssemblyDomain* pNewDomain = CreateDomain("GloryMain");
		if (!pNewDomain->SetCurrentDomain(true)) UnloadDomain("GloryDomain");
		m_pActiveDomain = pNewDomain;
	}

	void MonoManager::CollectGC()
	{
		mono_gc_collect(mono_gc_max_generation());
	}

	void MonoManager::CollectGC(int32_t generation)
	{
		mono_gc_collect(generation);
	}

	MonoManager* MonoManager::Instance()
	{
		return m_pInstance;
	}

	MonoManager::MonoManager(GloryMonoScipting* pModule)
		: m_pModule(pModule), m_pMethodsHelper(new ScriptingMethodsHelper()), m_pCoreLibManager(new CoreLibManager(this)),
		m_pRootDomain(nullptr), m_pActiveDomain(nullptr), m_HadInitialLoad(false)
	{
		m_pInstance = this;
	}

	MonoManager::~MonoManager()
	{
		delete m_pMethodsHelper;
		m_pMethodsHelper = nullptr;

		delete m_pCoreLibManager;
		m_pCoreLibManager = nullptr;

		m_pInstance = nullptr;
	}
}

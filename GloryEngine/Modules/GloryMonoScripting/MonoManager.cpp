#include "MonoManager.h"
#include "MonoSceneManager.h"
#include "GloryMonoScipting.h"
#include "MonoScriptObjectManager.h"
#include "CoreLibManager.h"
#include "ScriptingMethodsHelper.h"
#include "AssemblyDomain.h"
#include "MonoComponentObjectManager.h"

#include <Debug.h>

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

namespace Glory
{
	struct GMonoProfiler
	{
		static constexpr unsigned long long MaxGCAllocations = 250000;

		/* Handle obtained from mono_profiler_create (). */
		MonoProfilerHandle Handle;

		/* Counts the number of calls observed. */
		unsigned long long NCalls;

		/* Number of allocated bytes since last collection */
		unsigned long long ActiveGCBytes;

		/* Mono manager instance */
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

		Engine* pEngine = (Engine*)userData;

		std::stringstream str;
		str << "MONO: ";
		if (logDomain) str << "Domain: " << logDomain << ": ";
		if (logLevel) str << logLevel << ": ";
		if (message) str << message;
		if (fatal)
		{
			pEngine->GetDebug().LogFatalError(str.str());
			return;
		}
		pEngine->GetDebug().Log(str.str(), debugLogLevel);
	}

	void OnGCAllocation(MonoProfiler* pProfiler, MonoObject* obj)
	{
		// Get allocation info
		MonoClass* pMonoClass = mono_object_get_class(obj);
		const char* nameSpace = mono_class_get_namespace(pMonoClass);
		const char* name = mono_class_get_name(pMonoClass);
		const int32_t size = mono_class_instance_size(pMonoClass);

		GMonoProfiler* pGProfiler = (GMonoProfiler*)pProfiler;
		Engine* pEngine = pGProfiler->m_pMonoManager->Module()->GetEngine();
		const ModuleSettings& settings = pGProfiler->m_pMonoManager->Module()->Settings();
		const bool gcLogging = settings.Value<bool>("Enable GC allocation logging");
		if (gcLogging)
		{
			std::stringstream log;
			log << "New GC allocation of type " << nameSpace << "." << name << " with size " << size << " bytes";
			pEngine->GetDebug().LogInfo(log.str());
		}

		const bool autoGarbageCollect = settings.Value<bool>("Auto Collect Garbage");
		pGProfiler->ActiveGCBytes += size;
		if (pGProfiler->ActiveGCBytes < GMonoProfiler::MaxGCAllocations || !autoGarbageCollect) return;
		pGProfiler->m_pMonoManager->CollectGC();
		pGProfiler->ActiveGCBytes = 0;
	}

	void OnGCEvent(MonoProfiler* profiler, MonoProfilerGCEvent event, uint32_t generation, mono_bool isSerial)
	{
	}

	void OnPrintCallback(const char* string, mono_bool)
	{
		Engine* pEngine = MonoManager::Instance()->Module()->GetEngine();
		pEngine->GetDebug().LogWarning(string);
	}

	void OnPrintErrorCallback(const char* string, mono_bool)
	{
		Engine* pEngine = MonoManager::Instance()->Module()->GetEngine();
		pEngine->GetDebug().LogError(string);
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
        if (!Enum<MonoLogLevel>().ToString(logLevel, logLevelString))
            logLevelString = "error";
        mono_trace_set_level_string(logLevelString.c_str());

        /* Setup debugger */
        const std::string debugAgentIP = m_pModule->Settings().Value<std::string>("MonoDebuggingIP");
        const uint16_t debugAgentPort = m_pModule->Settings().Value<uint16_t>("MonoDebuggingPort");
		m_DebuggingEnabled = m_pModule->Settings().Value<bool>("Enable Debugging");

		if (m_DebuggingEnabled)
		{
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
		}

        /* Setup log callbacks */
        mono_trace_set_log_handler(OnLogCallback, Module()->GetEngine());
        mono_trace_set_print_handler(OnPrintCallback);
        mono_trace_set_printerr_handler(OnPrintErrorCallback);

		mono_config_parse(nullptr);

		GProfiler.m_pMonoManager = this;
		GProfiler.Handle = mono_profiler_create((MonoProfiler*)&GProfiler);
		mono_profiler_set_gc_allocation_callback(GProfiler.Handle, &OnGCAllocation);
		//mono_profiler_set_gc_event_callback(GProfiler.Handle, &OnGCEvent);
		mono_profiler_enable_allocations();

		const char* monoVersion = "v4.0.30319";

		MonoDomain* pMonoDomain = mono_jit_init_version("GloryRootDomain", monoVersion);
		m_pRootDomain = new AssemblyDomain("root", pMonoDomain);
		m_Domains.emplace("root", m_pRootDomain);
		m_pActiveDomain = m_pRootDomain;

		//mono_domain_set_config(m_pRootDomain, configDir.c_str(), configFilename.c_str());
		mono_thread_set_main(mono_thread_current());

		char* buildInfo = mono_get_runtime_build_info();
		std::stringstream buildInfoLog;
		buildInfoLog << "Mono Version: " << buildInfo;
		Module()->GetEngine()->GetDebug().LogInfo(buildInfoLog.str());
		mono_free(buildInfo);
	}

	void MonoManager::Cleanup()
	{
		m_pMethodsHelper->Cleanup();
		m_pCoreLibManager->Cleanup();

		for (auto& itor : m_Domains)
		{
			if (itor.first == "root") continue;
			UnloadDomain(itor.first, false);
		}
		m_Domains.clear();

		CollectGC();
		WaitForPendingFinalizers();

		m_pRootDomain->Unload(true);
		mono_jit_cleanup(m_pRootDomain->GetMonoDomain());
		delete m_pRootDomain;
		m_pRootDomain = nullptr;
		m_pActiveDomain = nullptr;
		
		if (m_DebuggingEnabled) mono_debug_cleanup();
	}

	void MonoManager::InitialLoad()
	{
		AssemblyDomain* pStartDomain = CreateDomain("GloryDomain");
		if (!pStartDomain->SetCurrentDomain())
		{
			Module()->GetEngine()->GetDebug().LogFatalError("MonoManager::InitialLoad > Failed to set initial domain");
			return;
		}
		m_pActiveDomain = pStartDomain;

		for (size_t i = 0; i < m_Libs.size(); ++i)
		{
			m_pActiveDomain->LoadLib(m_Libs[i]);
		}
		m_HadInitialLoad = true;

		m_ScriptExecutionAllowed = true;
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

	GloryMonoScipting* MonoManager::Module() const
	{
		return m_pModule;
	}

	CoreLibManager* MonoManager::GetCoreLibManager() const
	{
		return m_pCoreLibManager;
	}

	ScriptingMethodsHelper* MonoManager::GetMethodsHelper() const
	{
		return m_pMethodsHelper;
	}

	bool MonoManager::ScriptExecutionAllowed() const
	{
		return m_ScriptExecutionAllowed;
	}

	AssemblyDomain* MonoManager::CreateDomain(const std::string& name)
	{
		auto itor = m_Domains.find(name);
		if (itor != m_Domains.end()) return itor->second;

		const auto pMonoDomain = mono_domain_create_appdomain((char*)name.data(), nullptr);
		if (m_DebuggingEnabled) mono_debug_domain_create(pMonoDomain);
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
		pDomain->Unload(true);

		if (m_pActiveDomain == pDomain)
		{
			if (!m_pRootDomain->SetCurrentDomain())
			{
				Module()->GetEngine()->GetDebug().LogFatalError("MonoManager::UnloadDomain > Failed to set root domain as active!");
				return;
			}
			m_pActiveDomain = m_pRootDomain;
		}

		MonoObject* exception = nullptr;
		mono_domain_finalize(pDomain->GetMonoDomain(), 2000);
		mono_domain_try_unload(pDomain->GetMonoDomain(), &exception);
		//if (m_DebuggingEnabled) mono_debug_domain_unload(pDomain->GetMonoDomain());
		delete pDomain;
		
		if (!remove) return;
		m_Domains.erase(itor);
	}

	void MonoManager::Reload()
	{
		m_ScriptExecutionAllowed = false;

		UnloadDomain("GloryDomain");
		AssemblyDomain* pNewDomain = CreateDomain("GloryDomain");
		if (!pNewDomain->SetCurrentDomain())
		{
			Module()->GetEngine()->GetDebug().LogFatalError("MonoManager::Reload > Failed to set new domain as active");
			return;
		}
		m_pActiveDomain = pNewDomain;

		for (size_t i = 0; i < m_Libs.size(); ++i)
		{
			m_pActiveDomain->LoadLib(m_Libs[i]);
		}

		m_ScriptExecutionAllowed = true;
	}

	void MonoManager::CollectGC()
	{
		mono_gc_collect(mono_gc_max_generation());
	}

	void MonoManager::CollectGC(int32_t generation)
	{
		mono_gc_collect(generation);
	}

	void MonoManager::WaitForPendingFinalizers()
	{
		if (mono_gc_pending_finalizers())
		{
			mono_gc_finalize_notify();
			while (mono_gc_pending_finalizers())
			{
				std::this_thread::sleep_for(std::chrono::nanoseconds(1));
			}
		}
	}

	bool MonoManager::DebuggingEnabled() const
	{
		return m_DebuggingEnabled;
	}

	MonoManager* MonoManager::Instance()
	{
		return m_pInstance;
	}

	GloryMonoScipting* MonoManager::Module()
	{
		return m_pModule;
	}

	MonoManager::MonoManager(GloryMonoScipting* pModule)
		: m_pModule(pModule), m_pMethodsHelper(new ScriptingMethodsHelper()),
		m_pCoreLibManager(new CoreLibManager(this)), m_pRootDomain(nullptr), m_pActiveDomain(nullptr),
		m_HadInitialLoad(false), m_ScriptExecutionAllowed(false), m_DebuggingEnabled(false)
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

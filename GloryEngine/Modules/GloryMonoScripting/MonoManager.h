#pragma once
#include "GloryMono.h"

#include <string>

namespace Glory
{
	class CoreLibManager;
	class GloryMonoScipting;
	class ScriptingMethodsHelper;
	class AssemblyDomain;

	class MonoManager
	{
	public:
		GLORY_API void InitialLoad();

		GLORY_API void AddLib(const ScriptingLib& lib);

		GLORY_API GloryMonoScipting* Module() const;
		GLORY_API CoreLibManager* GetCoreLibManager() const;
		GLORY_API ScriptingMethodsHelper* GetMethodsHelper() const;
		GLORY_API bool ScriptExecutionAllowed() const;

		GLORY_API AssemblyDomain* AppDomain();

		GLORY_API void Reload();

		GLORY_API void CollectGC();
		GLORY_API void CollectGC(int32_t generation);
		GLORY_API void WaitForPendingFinalizers();
		GLORY_API void Reset(Engine* pEngine);
		GLORY_API bool DebuggingEnabled() const;

		static GLORY_API MonoManager* Instance();

		GLORY_API GloryMonoScipting* Module();

	private:
		void Initialize(const std::string& assemblyDir = ".", const std::string& configDir = "");
		void Cleanup();

	private:
		MonoManager(GloryMonoScipting* pModule);
		virtual ~MonoManager();

	private:
		friend class GloryMonoScipting;
		static MonoManager* m_pInstance;
		GloryMonoScipting* m_pModule;
		CoreLibManager* m_pCoreLibManager;
		ScriptingMethodsHelper* m_pMethodsHelper;
		AssemblyDomain* m_pRootDomain;
		AssemblyDomain* m_pAppDomain;

		bool m_DebuggingEnabled;

		std::vector<ScriptingLib> m_Libs;

		bool m_HadInitialLoad;
		bool m_ScriptExecutionAllowed;
	};
}

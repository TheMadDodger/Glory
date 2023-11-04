#pragma once
#include <string>
#include <ScriptingExtender.h>
#include "GloryMono.h"

namespace Glory
{
	class CoreLibManager;
	class EntityLibManager;
	class GloryMonoScipting;
	class ScriptingMethodsHelper;
	class AssemblyDomain;

	class MonoManager
	{
	public:
		GLORY_API void InitialLoad();

		GLORY_API AssemblyDomain* GetDomain(const std::string& name);
		GLORY_API void AddLib(const ScriptingLib& lib);

		GLORY_API GloryMonoScipting* Module() const;
		GLORY_API CoreLibManager* GetCoreLibManager() const;
		GLORY_API EntityLibManager* GetEntityLibManager() const;
		GLORY_API ScriptingMethodsHelper* GetMethodsHelper() const;
		GLORY_API bool ScriptExecutionAllowed() const;

		GLORY_API AssemblyDomain* CreateDomain(const std::string& name);
		GLORY_API AssemblyDomain* ActiveDomain();
		GLORY_API void UnloadDomain(const std::string& name, bool remove = true);

		GLORY_API void Reload();

		GLORY_API void CollectGC();
		GLORY_API void CollectGC(int32_t generation);
		GLORY_API void WaitForPendingFinalizers();
		GLORY_API bool DebuggingEnabled() const;

		static GLORY_API MonoManager* Instance();

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
		EntityLibManager* m_pEntityLibManager;
		ScriptingMethodsHelper* m_pMethodsHelper;
		std::map<std::string, AssemblyDomain*> m_Domains;
		AssemblyDomain* m_pRootDomain;
		AssemblyDomain* m_pActiveDomain;

		bool m_DebuggingEnabled;

		std::vector<ScriptingLib> m_Libs;

		bool m_HadInitialLoad;
		bool m_ScriptExecutionAllowed;
	};
}

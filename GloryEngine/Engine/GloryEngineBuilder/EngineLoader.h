#pragma once
#include <Engine.h>
#include <Window.h>
#include <Module.h>

#include <string>
#include <filesystem>
#include <windows.h>

namespace Glory
{
	class Console;
	class Debug;

	class EngineLoader
	{
	public:
		EngineLoader(const std::filesystem::path& cfgPath);
		virtual ~EngineLoader();

		Engine LoadEngine(Console* pConsole, Debug* pDebug);
		Engine LoadEngineFromPath(Console* pConsole, Debug* pDebug);
		EngineCreateInfo LoadEngineInfoFromPath(Console* pConsole, Debug* pDebug);
		void Unload();

		const size_t ModuleCount() const;
		const Module* GetModule(size_t index) const;

	private:
		void LoadModules(YAML::Node& modules);
		void LoadModule(std::string& moduleName);
		void ReadModule(Module* pModule);
		void LoadExtras();
		void LoadExtra(const std::string& name, const std::filesystem::path& path, Module* pModule, Module* pRequiredModule);

	private:
		const std::filesystem::path m_CFGPath;
		std::vector<HMODULE> m_DependencyLibs;
		std::vector<HMODULE> m_ModuleLibs;
		std::vector<HMODULE> m_ExtraLibs;
		std::vector<Module*> m_pMainModules;
		std::vector<Module*> m_pAllModules;
		std::vector<std::string> m_LoadedModuleNames;

		EngineCreateInfo m_EngineInfo;
		std::vector<Module*> m_pOptionalModules;
	};
}

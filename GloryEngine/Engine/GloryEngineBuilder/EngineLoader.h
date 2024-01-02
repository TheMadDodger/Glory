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
		EngineLoader(const std::filesystem::path& cfgPath, const Glory::WindowCreateInfo& defaultWindow);
		virtual ~EngineLoader();

		Engine LoadEngine(Console* pConsole, Debug* pDebug);
		void Unload();

		const size_t ModuleCount() const;
		const Module* GetModule(size_t index) const;

	private:
		void LoadModules(YAML::Node& modules);
		void LoadModule(const std::string& moduleName);
		void ReadModule(Module* pModule);

		void LoadScriptingExtendersForScripting(ScriptingModule* pModule);
		void LoadScriptingExtender(const ModuleScriptingExtension* const extension, Module* pModule, const ModuleMetaData& metaData);

	private:
		const std::filesystem::path m_CFGPath;
		std::vector<HMODULE> m_DependencyLibs;
		std::vector<HMODULE> m_ModuleLibs;
		std::vector<HMODULE> m_ScriptingLibs;
		std::vector<Module*> m_pMainModules;
		std::vector<Module*> m_pAllModules;
		std::vector<std::string> m_LoadedModuleNames;

		EngineCreateInfo m_EngineInfo;
		const Glory::WindowCreateInfo m_DefaultWindow;
		std::vector<Module*> m_pOptionalModules;
		std::vector<ScriptingModule*> m_pScriptingModules;
		std::vector<IScriptExtender*> m_pScriptingExtenders;
	};
}

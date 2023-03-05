#pragma once
#include <Engine.h>
#include <Module.h>

#include <string>
#include <filesystem>
#include <windows.h>

namespace Glory
{
	class EngineLoader
	{
	public:
		EngineLoader(const std::filesystem::path& cfgPath);
		virtual ~EngineLoader();

		Engine* LoadEngine(const Glory::WindowCreateInfo& defaultWindow);
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
		std::vector<Module*> m_pModules;
		std::vector<std::string> m_LoadedModuleNames;

		EngineCreateInfo m_EngineInfo;
		std::vector<Module*> m_pOptionalModules;
		std::vector<ScriptingModule*> m_pScriptingModules;
		std::vector<IScriptExtender*> m_pScriptingExtenders;
	};
}

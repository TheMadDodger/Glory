#pragma once
#include <Engine.h>
#include <string>
#include <filesystem>
#include <windows.h>
#include <RendererModule.h>

namespace Glory
{
	class EngineLoader
	{
	public:
		EngineLoader(const std::filesystem::path& cfgPath);
		virtual ~EngineLoader();

		Engine* LoadEngine(const Glory::WindowCreateInfo& defaultWindow);
		void Unload();

		const std::string& GetSetModule(const std::string& key);

		const size_t ModuleCount() const;
		const Module* GetModule(size_t index) const;

	private:
		void LoadModules(YAML::Node& modules);
		void LoadModule(const std::string& moduleName);
		void PopulateEngineInfo(YAML::Node& engineInfo, EngineCreateInfo& engineCreateInfo, const Glory::WindowCreateInfo& defaultWindow);

		template<class T>
		T* LoadRequiredModule(YAML::Node& node, const std::string& key, T** value)
		{
			YAML::Node indexNode = node[key];
			size_t index = indexNode.as<size_t>();
			Module* pModule = m_pModules[index];
			T* pT = (T*)pModule;
			*value = pT;
			m_SetModules[key] = index;
			return pT;
		}

		void LoadScriptingModules(YAML::Node& node, const std::string& key, EngineCreateInfo& engineCreateInfo);

	private:
		std::filesystem::path m_CFGPath;
		std::vector<Module*> m_pModules;
		std::vector<HMODULE> m_Libs;
		std::vector<Module*> m_pOptionalModules;
		std::vector<ScriptingModule*> m_pScriptingModules;
		std::vector<std::string> m_LoadedModuleNames;
		std::map<std::string, size_t> m_SetModules;
	};
}

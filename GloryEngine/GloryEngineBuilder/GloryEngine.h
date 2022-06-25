#pragma once
#include <Engine.h>
#include <string>
#include <filesystem>

namespace Glory
{
	class EngineLoader
	{
	public:
		EngineLoader(const std::filesystem::path& cfgPath);
		virtual ~EngineLoader();

		Engine* LoadEngine();
		void Unload();

	private:
		void LoadModules(YAML::Node& modules);
		void LoadModule(const std::string& moduleName);
		void PopulateEngineInfo(YAML::Node& engineInfo, EngineCreateInfo& engineCreateInfo);

		template<class T>
		void LoadRequiredModule(YAML::Node& node, const std::string& key, T** value)
		{
			YAML::Node indexNode = node[key];
			int index = indexNode.as<int>();
			Module* pModule = m_pModules[index];
			T* pT = (T*)pModule;
			*value = pT;
		}

	private:
		std::filesystem::path m_CFGPath;
		std::vector<Module*> m_pModules;
		std::vector<HMODULE> m_Libs;
	};
}
#include "GloryEngine.h"
#include <yaml-cpp/yaml.h>
#include <windows.h>
#include <Debug.h>

namespace Glory
{
	typedef Module*(__cdecl* LoadModuleProc)();

	EngineLoader::EngineLoader(const std::filesystem::path& cfgPath) : m_CFGPath(cfgPath)
	{
	}

	EngineLoader::~EngineLoader()
	{
	}

	Engine* EngineLoader::LoadEngine()
	{
		YAML::Node node = YAML::LoadFile(m_CFGPath.string());
		YAML::Node modules = node["Modules"];
		LoadModules(modules);
		YAML::Node engineInfo = node["Engine"];
		EngineCreateInfo engineCreateInfo{};
		PopulateEngineInfo(engineInfo, engineCreateInfo);
		return Engine::CreateEngine(engineCreateInfo);
	}

	void EngineLoader::Unload()
	{
		m_pModules.clear();
		for (size_t i = 0; i < m_Libs.size(); i++)
		{
			FreeLibrary(m_Libs[i]);
		}
		m_Libs.clear();
	}

	void EngineLoader::LoadModules(YAML::Node& modules)
	{
		for (size_t i = 0; i < modules.size(); i++)
		{
			YAML::Node moduleNode = modules[i];
			std::string moduleName = moduleNode.as<std::string>();
			LoadModule(moduleName);
		}
	}

	void EngineLoader::LoadModule(const std::string& moduleName)
	{
		std::filesystem::path modulePath = "./Modules";
		std::filesystem::path dllPath = modulePath.append(moduleName);
		dllPath = dllPath.append(moduleName).replace_extension(".dll");

		HMODULE lib = LoadLibrary(dllPath.wstring().c_str());
		if (lib == NULL)
		{
			Debug::LogFatalError("Missing module: " + moduleName);
			return;
		}

		LoadModuleProc loadProc = (LoadModuleProc)GetProcAddress(lib, "LoadModule");
		if (loadProc == NULL)
		{
			FreeLibrary(lib);
			Debug::LogFatalError("Module was loaded with missing LoadModule function: " + moduleName);
			return;
		}

		Module* pModule = (loadProc)();
		if (pModule == nullptr)
		{
			FreeLibrary(lib);
			Debug::LogFatalError("Module was loaded but the LoadModule function returned a nullptr: " + moduleName);
			return;
		}
		m_pModules.push_back(pModule);
		m_Libs.push_back(lib);
	}

	void EngineLoader::PopulateEngineInfo(YAML::Node& engineInfo, EngineCreateInfo& engineCreateInfo)
	{
		LoadRequiredModule<ScenesModule>(engineInfo, "Scenes", &engineCreateInfo.pScenesModule);
	}
}

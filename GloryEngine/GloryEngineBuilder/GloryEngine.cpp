#include "GloryEngine.h"
#include <yaml-cpp/yaml.h>
#include <Debug.h>
#include <BuiltInModules.h>
#include <Console.h>
#include <WindowsDebugConsole.h>

namespace Glory
{
	typedef Module*(__cdecl* LoadModuleProc)();

	EngineLoader::EngineLoader(const std::filesystem::path& cfgPath) : m_CFGPath(cfgPath)
	{
	}

	EngineLoader::~EngineLoader()
	{
	}

	Engine* EngineLoader::LoadEngine(const Glory::WindowCreateInfo& defaultWindow)
	{
		Console::Initialize();

#ifdef _DEBUG
		Console::RegisterConsole<WindowsDebugConsole>();
#endif

		YAML::Node node = YAML::LoadFile(m_CFGPath.string());
		YAML::Node modules = node["Modules"];
		LoadModules(modules);
		YAML::Node engineInfo = node["Engine"];
		EngineCreateInfo engineCreateInfo{};
		PopulateEngineInfo(engineInfo, engineCreateInfo, defaultWindow);
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

		Console::Cleanup();
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
		Debug::LogInfo("Loading module: " + moduleName + "...");

		std::filesystem::path modulePath = "./Modules";
		std::filesystem::path dllPath = modulePath.append(moduleName);
		dllPath = dllPath.append(moduleName).replace_extension(".dll");

		HMODULE lib = LoadLibrary(dllPath.wstring().c_str());
		if (lib == NULL)
		{
			Debug::LogError("Failed to load module: " + moduleName + ": The module was not found!");
			m_pModules.push_back(nullptr);
			return;
		}

		LoadModuleProc loadProc = (LoadModuleProc)GetProcAddress(lib, "LoadModule");
		if (loadProc == NULL)
		{
			FreeLibrary(lib);
			Debug::LogError("Failed to load module: " + moduleName + ": Missing LoadModule function!");
			m_pModules.push_back(nullptr);
			return;
		}

		Module* pModule = (loadProc)();
		if (pModule == nullptr)
		{
			FreeLibrary(lib);
			Debug::LogError("Failed to load module: " + moduleName + ": LoadModule returned nullptr!");
			m_pModules.push_back(nullptr);
			return;
		}
		m_pModules.push_back(pModule);
		m_Libs.push_back(lib);
	}

	void EngineLoader::PopulateEngineInfo(YAML::Node& engineInfo, EngineCreateInfo& engineCreateInfo, const Glory::WindowCreateInfo& defaultWindow)
	{
		LoadRequiredModule<WindowModule>(engineInfo, "Window", &engineCreateInfo.pWindowModule)->SetMainWindowCreateInfo(defaultWindow);
		LoadRequiredModule<ScenesModule>(engineInfo, "Scenes", &engineCreateInfo.pScenesModule);
		engineCreateInfo.pRenderModule = new NullRenderer();
		//LoadRequiredModule<RendererModule>(engineInfo, "Renderer", &engineCreateInfo.pRenderModule);
		LoadRequiredModule<GraphicsModule>(engineInfo, "Graphics", &engineCreateInfo.pGraphicsModule);

		YAML::Node optionalModulesNode = engineInfo["Optional"];
		for (size_t i = 0; i < optionalModulesNode.size(); i++)
		{
			YAML::Node indexNode = optionalModulesNode[i];
			int index = indexNode.as<int>();
			Module* pModule = m_pModules[index];
			m_pOptionalModules.push_back(pModule);
		}

		m_pOptionalModules.push_back(new Glory::FileLoaderModule());
		m_pOptionalModules.push_back(new Glory::MaterialLoaderModule());
		m_pOptionalModules.push_back(new Glory::MaterialInstanceLoaderModule());
		m_pOptionalModules.push_back(new Glory::ShaderSourceLoaderModule());

		engineCreateInfo.OptionalModuleCount = static_cast<uint32_t>(m_pOptionalModules.size());
		engineCreateInfo.pOptionalModules = m_pOptionalModules.data();
	}
}

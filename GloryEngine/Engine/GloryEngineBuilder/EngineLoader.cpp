#include "EngineLoader.h"

#include <GloryContext.h>
#include <BuiltInModules.h>

std::string GetLastErrorAsString()
{
	//Get the error message ID, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) {
		return std::string(); //No error message has been recorded
	}

	LPSTR messageBuffer = nullptr;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	std::string message(messageBuffer, size);

	//Free the Win32's string's buffer.
	LocalFree(messageBuffer);

	return message;
}

namespace Glory
{
	typedef Module* (__cdecl* OnLoadModuleProc)(GloryContext*);
	typedef IScriptExtender*(__cdecl* OnLoadExtensionProc)(GloryContext*);
	
	EngineLoader::EngineLoader(const std::filesystem::path& cfgPath)
		: m_CFGPath(cfgPath) {}
	
	EngineLoader::~EngineLoader() {}

	Engine* EngineLoader::LoadEngine(const Glory::WindowCreateInfo& defaultWindow)
	{
		if (!std::filesystem::exists(m_CFGPath))
		{
			Debug::LogError("Missing Engine.yaml file in project directory!");
			return nullptr;
		}
		
		m_EngineInfo = {};
		YAML::Node node = YAML::LoadFile(m_CFGPath.string());
		YAML::Node modules = node["Modules"];
		LoadModules(modules);
		YAML::Node engineInfo = node["Engine"];

		m_pOptionalModules.push_back(new Glory::FileLoaderModule());
		m_pOptionalModules.push_back(new Glory::MaterialLoaderModule());
		m_pOptionalModules.push_back(new Glory::MaterialInstanceLoaderModule());
		m_pOptionalModules.push_back(new Glory::TextureDataLoaderModule());
		m_pOptionalModules.push_back(new Glory::ShaderSourceLoaderModule());

		for (size_t i = 0; i < m_pScriptingModules.size(); i++)
		{
			LoadScriptingExtendersForScripting(m_pScriptingModules[i]);
		}

		m_EngineInfo.OptionalModuleCount = static_cast<uint32_t>(m_pOptionalModules.size());
		m_EngineInfo.pOptionalModules = m_pOptionalModules.data();

		m_EngineInfo.ScriptingModulesCount = static_cast<uint32_t>(m_pScriptingModules.size());
		m_EngineInfo.pScriptingModules = m_pScriptingModules.data();

		if (m_EngineInfo.pWindowModule) m_EngineInfo.pWindowModule->SetMainWindowCreateInfo(defaultWindow);
		return Engine::CreateEngine(m_EngineInfo);
	}
		
	void EngineLoader::Unload()
	{
		m_pModules.clear();
		m_LoadedModuleNames.clear();
		m_pOptionalModules.clear();
		m_pScriptingModules.clear();
		m_pScriptingExtenders.clear();

		/* Unload scripting libs */
		for (size_t i = 0; i < m_ScriptingLibs.size(); i++)
		{
			FreeLibrary(m_ScriptingLibs[i]);
		}
		m_ScriptingLibs.clear();
		
		/* Unload module libs */
		for (size_t i = 0; i < m_ModuleLibs.size(); i++)
		{
			FreeLibrary(m_ModuleLibs[i]);
		}
		m_ModuleLibs.clear();

		/* Unload dependency libs */
		for (size_t i = 0; i < m_DependencyLibs.size(); i++)
		{
			FreeLibrary(m_DependencyLibs[i]);
		}
		m_DependencyLibs.clear();
	}

	const size_t EngineLoader::ModuleCount() const
	{
		return m_pModules.size();
	}
		
	const Module* EngineLoader::GetModule(size_t index) const
	{
		return m_pModules[index];
	}

	void EngineLoader::LoadModules(YAML::Node& modules)
	{
		size_t modulesCount = modules.size();
		for (size_t i = 0; i < modulesCount; i++)
		{
			YAML::Node moduleNode = modules[i];
			std::string moduleName = moduleNode.as<std::string>();
			LoadModule(moduleName);
		}
	}

	void EngineLoader::LoadModule(const std::string& moduleName)
	{
		std::stringstream debugStream;

		std::filesystem::path modulePath = "./Modules";
		modulePath.append(moduleName);

		/* Read meta data */
		Debug::LogInfo("Loading module metadata: " + moduleName + "...");
		std::filesystem::path metaPath = modulePath;
		metaPath = metaPath.append("Module.yaml");
		ModuleMetaData metaData(metaPath);
		metaData.Read();

		/* Load dependencies */
		const std::vector<std::string>& dependencies = metaData.Dependencies();
		size_t dependencyCount = dependencies.size();
		if (dependencyCount > 0)
		{
			Debug::LogInfo("Loading dependencies for module: " + moduleName + "...");
			for (size_t i = 0; i < dependencyCount; i++)
			{
				std::string dependency = dependencies[i];
				Debug::LogInfo("Loading dependency: " + dependency + "...");
				std::filesystem::path dependencyPath = modulePath;
				dependencyPath = dependencyPath.append("Dependencies").append(dependency);
				dependencyPath = dependencyPath.string() + ".dll";

				if (!std::filesystem::exists(dependencyPath))
				{
#if _DEBUG
					/* Search for debug lib instead */
					std::filesystem::path debugPath = dependencyPath.filename().replace_extension().string() + 'd';
					debugPath.replace_extension(".dll");
					dependencyPath.replace_filename(debugPath);
					if (!std::filesystem::exists(dependencyPath))
					{
						debugStream.clear();
						debugStream << "Failed to load dependency: " << dependency << ": The library was not found!";
						Debug::LogError(debugStream.str());
						return;
					}
#else
					debugStream.clear();
					debugStream << "Failed to load dependency: " << dependency << ": The library was not found!";
					Debug::LogError(debugStream.str());
					return;
#endif
				}

				HMODULE dependencyLib = LoadLibrary(dependencyPath.c_str());
				if (dependencyLib == NULL)
				{
					debugStream.clear();
					debugStream << "Failed to load dependency: " << dependency << ": There was an error while loading the library!";
					Debug::LogError(debugStream.str());
					Debug::LogError(GetLastErrorAsString());
					return;
				}
				m_DependencyLibs.push_back(dependencyLib);
			}
		}

		/* Load module lib */
		debugStream.clear();
		debugStream << "Loading module: " << moduleName << "...";
		Debug::LogInfo(debugStream.str());

		std::filesystem::path dllPath = modulePath;
		dllPath = dllPath.append(moduleName).replace_extension(".dll");
		if (!std::filesystem::exists(dllPath))
		{
			debugStream.clear();
			debugStream << "Failed to load module: " << moduleName << ": There was an error while loading the library!";
			Debug::LogError(debugStream.str());
			return;
		}

		HMODULE lib = LoadLibrary(dllPath.wstring().c_str());
		if (lib == NULL)
		{
			debugStream.clear();
			debugStream << "Failed to load module: " << moduleName << ": The module was not found!";
			Debug::LogError(debugStream.str());
			Debug::LogError(GetLastErrorAsString());
			return;
		}

		OnLoadModuleProc loadProc = (OnLoadModuleProc)GetProcAddress(lib, "OnLoadModule");
		if (loadProc == NULL)
		{
			FreeLibrary(lib);
			Debug::LogError("Failed to load module: " + moduleName + ": Missing OnLoadModule function!");
			return;
		}

		GloryContext* pContext = GloryContext::GetContext();
		Module* pModule = (loadProc)(pContext);
		if (pModule == nullptr)
		{
			FreeLibrary(lib);
			Debug::LogError("Failed to load module: " + moduleName + ": OnLoadModule returned nullptr!");
			return;
		}

		m_ModuleLibs.push_back(lib);
		pModule->SetMetaData(metaData);
		m_pModules.push_back(pModule);
		m_LoadedModuleNames.push_back(moduleName);
		ReadModule(pModule);
	}

	void EngineLoader::ReadModule(Module* pModule)
	{
		const ModuleMetaData& metaData = pModule->GetMetaData();

		ModuleType moduleType = metaData.Type();
		switch (moduleType)
		{
		case Glory::ModuleType::MT_Window:
			if (m_EngineInfo.pWindowModule != nullptr)
			{
				Debug::LogWarning("Multiple window modules is not supported");
				return;
			}
			m_EngineInfo.pWindowModule = static_cast<WindowModule*>(pModule);
			break;
		case Glory::ModuleType::MT_Graphics:
			if (m_EngineInfo.pGraphicsModule != nullptr)
			{
				Debug::LogWarning("Multiple graphics modules is currently not supported");
				return;
			}
			m_EngineInfo.pGraphicsModule = static_cast<GraphicsModule*>(pModule);
			break;
		case Glory::ModuleType::MT_Renderer:
			if (m_EngineInfo.pRenderModule != nullptr)
			{
				Debug::LogWarning("Multiple renderer modules is currently not supported");
				return;
			}
			m_EngineInfo.pRenderModule = static_cast<RendererModule*>(pModule);
			break;
		case Glory::ModuleType::MT_SceneManagement:
			if (m_EngineInfo.pScenesModule != nullptr)
			{
				Debug::LogWarning("Multiple scene management modules is currently not supported");
				return;
			}
			m_EngineInfo.pScenesModule = static_cast<ScenesModule*>(pModule);
			break;
		case Glory::ModuleType::MT_Scripting:
			m_pScriptingModules.push_back(static_cast<ScriptingModule*>(pModule));
			break;
		case Glory::ModuleType::MT_Input:
			if (m_EngineInfo.pInputModule != nullptr)
			{
				Debug::LogWarning("Multiple input modules is currently not supported");
				return;
			}
			m_EngineInfo.pInputModule = static_cast<InputModule*>(pModule);
			break;
		case Glory::ModuleType::MT_Physics:
			if (m_EngineInfo.pPhysicsModule != nullptr)
			{
				Debug::LogWarning("Multiple physics modules is not supported");
				return;
			}
			m_EngineInfo.pPhysicsModule = static_cast<PhysicsModule*>(pModule);
			break;

		case Glory::ModuleType::MT_Loader:
		case Glory::ModuleType::MT_Other:
			m_pOptionalModules.push_back(pModule);
			break;
		default:
			break;
		}
	}

	void EngineLoader::LoadScriptingExtendersForScripting(ScriptingModule* pScriptingModule)
	{
		for (size_t i = 0; i < m_pModules.size(); i++)
		{
			Module* pModule = m_pModules[i];
			const ModuleMetaData& metaData = pModule->GetMetaData();
			const ModuleScriptingExtension* const extender = metaData.ScriptExtenderForLanguage(pScriptingModule->ScriptingLanguage());
			if (extender == nullptr) continue;
			LoadScriptingExtender(extender, pModule, metaData);
		}
	}

	void EngineLoader::LoadScriptingExtender(const ModuleScriptingExtension* const extension, Module* pScriptingModule, const ModuleMetaData& metaData)
	{
		std::filesystem::path pathToExtension = metaData.Path();
		pathToExtension = pathToExtension.parent_path().append("Scripting").append(extension->m_Language).append(extension->m_ExtensionFile).replace_extension(".dll");
		
		Debug::LogInfo("Loading scripting extender " + extension->m_ExtensionFile + " for language " + extension->m_Language + "...");
		HMODULE lib = LoadLibrary(pathToExtension.wstring().c_str());
		if (lib == NULL)
		{
			Debug::LogError("Failed to load scripting extender: " + extension->m_ExtensionFile + ": The extension was not found!");
			return;
		}
		
		OnLoadExtensionProc loadProc = (OnLoadExtensionProc)GetProcAddress(lib, "OnLoadExtension");
		if (loadProc == NULL)
		{
			FreeLibrary(lib);
			Debug::LogError("Failed to load scripting extender: " + extension->m_ExtensionFile + ": Missing OnLoadExtension function!");
			return;
		}
		
		IScriptExtender* pScriptExtender = loadProc(GloryContext::GetContext());
		if (pScriptExtender == nullptr)
		{
			FreeLibrary(lib);
			Debug::LogError("Failed to load scripting extender: " + extension->m_ExtensionFile + ": OnLoadExtension returned nullptr!");
			return;
		}
		
		pScriptingModule->AddScriptingExtender(pScriptExtender);
		m_pScriptingExtenders.push_back(pScriptExtender);
		m_ScriptingLibs.push_back(lib);
	}
}

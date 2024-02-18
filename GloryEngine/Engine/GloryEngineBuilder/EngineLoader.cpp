#include "EngineLoader.h"

#include <Debug.h>
#include <WindowModule.h>
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
	typedef Module* (__cdecl* OnLoadModuleProc)();
	typedef bool(__cdecl* OnLoadExtraProc)(const char*, Module*, Module*);
	
	EngineLoader::EngineLoader(const std::filesystem::path& cfgPath, const Glory::WindowCreateInfo& defaultWindow)
		: m_CFGPath(cfgPath), m_DefaultWindow(defaultWindow), m_EngineInfo{} {}
	
	EngineLoader::~EngineLoader()
	{
		Unload();
	}

	Engine EngineLoader::LoadEngine(Console* pConsole, Debug* pDebug)
	{
		if (!std::filesystem::exists(m_CFGPath))
		{
			pDebug->LogFatalError("Missing Engine.yaml file in project directory!");
			throw std::exception();
		}
		
		m_EngineInfo = {};
		m_EngineInfo.m_pConsole = pConsole;
		m_EngineInfo.m_pDebug = pDebug;

		YAML::Node node = YAML::LoadFile(m_CFGPath.string());
		YAML::Node modules = node["Modules"];
		LoadModules(modules);

		m_pOptionalModules.push_back(new Glory::FileLoaderModule());
		m_pOptionalModules.push_back(new Glory::TextureDataLoaderModule());

		m_EngineInfo.MainModuleCount = static_cast<uint32_t>(m_pMainModules.size());
		m_EngineInfo.pMainModules = m_pMainModules.data();

		m_EngineInfo.OptionalModuleCount = static_cast<uint32_t>(m_pOptionalModules.size());
		m_EngineInfo.pOptionalModules = m_pOptionalModules.data();

		return { m_EngineInfo };
	}

	Engine EngineLoader::LoadEngineFromPath(Console* pConsole, Debug* pDebug)
	{
		if (!std::filesystem::exists(m_CFGPath))
		{
			pDebug->LogFatalError("Provided path is invalid!");
			throw std::exception();
		}

		m_EngineInfo = {};
		m_EngineInfo.m_pConsole = pConsole;
		m_EngineInfo.m_pDebug = pDebug;

		/* Find and load modules at provided path */
		for (auto itor : std::filesystem::directory_iterator(m_CFGPath))
		{
			const std::filesystem::path dir = itor.path();
			const std::string dirString = dir.string();
			if (!itor.is_directory()) continue;
			const std::string name = dir.filename().string();
			std::filesystem::path dllPath = dir;
			dllPath.append(name).replace_extension("dll");
			if (std::filesystem::exists(dllPath))
			{
				LoadModule(name);
			}
		}

		/* Sort modules */
		std::sort(m_pMainModules.begin(), m_pMainModules.end(), [](Module* pModuleA, Module* pModuleB)
		{
			const ModuleMetaData& metaA = pModuleA->GetMetaData();
			const ModuleMetaData& metaB = pModuleB->GetMetaData();
			const ModuleType typeA = metaA.Type();
			const ModuleType typeB = metaB.Type();
			return typeA < typeB;
		});

		/* Load extras */
		LoadExtras();

		m_pOptionalModules.push_back(new Glory::FileLoaderModule());
		m_pOptionalModules.push_back(new Glory::TextureDataLoaderModule());

		m_EngineInfo.MainModuleCount = static_cast<uint32_t>(m_pMainModules.size());
		m_EngineInfo.pMainModules = m_pMainModules.data();

		m_EngineInfo.OptionalModuleCount = static_cast<uint32_t>(m_pOptionalModules.size());
		m_EngineInfo.pOptionalModules = m_pOptionalModules.data();

		return { m_EngineInfo };
	}
		
	void EngineLoader::Unload()
	{
		m_pAllModules.clear();
		m_LoadedModuleNames.clear();
		m_pOptionalModules.clear();

		/* Unload other libs */
		for (size_t i = 0; i < m_ExtraLibs.size(); ++i)
		{
			FreeLibrary(m_ExtraLibs[i]);
		}
		m_ModuleLibs.clear();
		
		/* Unload module libs */
		for (size_t i = 0; i < m_ModuleLibs.size(); ++i)
		{
			FreeLibrary(m_ModuleLibs[i]);
		}
		m_ModuleLibs.clear();

		/* Unload dependency libs */
		for (size_t i = 0; i < m_DependencyLibs.size(); ++i)
		{
			FreeLibrary(m_DependencyLibs[i]);
		}
		m_DependencyLibs.clear();
	}

	const size_t EngineLoader::ModuleCount() const
	{
		return m_pAllModules.size();
	}
		
	const Module* EngineLoader::GetModule(size_t index) const
	{
		return m_pAllModules[index];
	}

	void EngineLoader::LoadModules(YAML::Node& modules)
	{
		size_t modulesCount = modules.size();
		for (size_t i = 0; i < modulesCount; ++i)
		{
			YAML::Node moduleNode = modules[i];
			std::string moduleName = moduleNode.as<std::string>();
			LoadModule(moduleName);
		}

		std::sort(m_pMainModules.begin(), m_pMainModules.end(), [](Module* pModuleA, Module* pModuleB)
		{
			const ModuleMetaData& metaA = pModuleA->GetMetaData();
			const ModuleMetaData& metaB = pModuleB->GetMetaData();
			const ModuleType typeA = metaA.Type();
			const ModuleType typeB = metaB.Type();
			return typeA < typeB;
		});

		/* Load extras */
		LoadExtras();
	}

	void EngineLoader::LoadModule(const std::string& moduleName)
	{
		if (moduleName == "GloryEntityScenes")
		{
			m_EngineInfo.m_pDebug->LogInfo("GloryEntityScenes has been removed and is now part of the core engine");
			return;
		}

		std::stringstream debugStream;

		std::filesystem::path modulePath = "./Modules";
		modulePath.append(moduleName);

		/* Read meta data */
		m_EngineInfo.m_pDebug->LogInfo("Loading module metadata: " + moduleName + "...");
		std::filesystem::path metaPath = modulePath;
		metaPath = metaPath.append("Module.yaml");
		ModuleMetaData metaData(metaPath);
		metaData.Read();

		if (metaData.Type() == ModuleType::MT_Invalid)
		{
			m_EngineInfo.m_pDebug->LogError("Failed to load module " + moduleName + " because the module is invalid!");
			return;
		}

		/* Load dependencies */
		const std::vector<std::string>& dependencies = metaData.Dependencies();
		size_t dependencyCount = dependencies.size();
		if (dependencyCount > 0)
		{
			m_EngineInfo.m_pDebug->LogInfo("Loading dependencies for module: " + moduleName + "...");
			for (size_t i = 0; i < dependencyCount; i++)
			{
				std::string dependency = dependencies[i];
				m_EngineInfo.m_pDebug->LogInfo("Loading dependency: " + dependency + "...");
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
						m_EngineInfo.m_pDebug->LogError(debugStream.str());
						return;
					}
#else
					debugStream.clear();
					debugStream << "Failed to load dependency: " << dependency << ": The library was not found!";
					m_EngineInfo.m_pDebug->LogError(debugStream.str());
					return;
#endif
				}

				HMODULE dependencyLib = LoadLibrary(dependencyPath.c_str());
				if (dependencyLib == NULL)
				{
					debugStream.clear();
					debugStream << "Failed to load dependency: " << dependency << ": There was an error while loading the library!";
					m_EngineInfo.m_pDebug->LogError(debugStream.str());
					m_EngineInfo.m_pDebug->LogError(GetLastErrorAsString());
					return;
				}
				m_DependencyLibs.push_back(dependencyLib);
			}
		}

		/* Load module lib */
		debugStream.clear();
		debugStream << "Loading module: " << moduleName << "...";
		m_EngineInfo.m_pDebug->LogInfo(debugStream.str());

		std::filesystem::path dllPath = modulePath;
		dllPath = dllPath.append(moduleName).replace_extension(".dll");
		if (!std::filesystem::exists(dllPath))
		{
			debugStream.clear();
			debugStream << "Failed to load module: " << moduleName << ": The module was not found!";
			m_EngineInfo.m_pDebug->LogError(debugStream.str());
			return;
		}

		HMODULE lib = LoadLibrary(dllPath.wstring().c_str());
		if (lib == NULL)
		{
			debugStream.clear();
			debugStream << "Failed to load module: " << moduleName << ": There was an error while loading the library!";
			m_EngineInfo.m_pDebug->LogError(debugStream.str());
			m_EngineInfo.m_pDebug->LogError(GetLastErrorAsString());
			return;
		}

		OnLoadModuleProc loadProc = (OnLoadModuleProc)GetProcAddress(lib, "OnLoadModule");
		if (loadProc == NULL)
		{
			FreeLibrary(lib);
			m_EngineInfo.m_pDebug->LogError("Failed to load module: " + moduleName + ": Missing OnLoadModule function!");
			return;
		}

		Module* pModule = (loadProc)();
		if (pModule == nullptr)
		{
			FreeLibrary(lib);
			m_EngineInfo.m_pDebug->LogError("Failed to load module: " + moduleName + ": OnLoadModule returned nullptr!");
			return;
		}

		m_ModuleLibs.push_back(lib);
		pModule->SetMetaData(metaData);
		m_pAllModules.push_back(pModule);
		m_LoadedModuleNames.push_back(moduleName);
		ReadModule(pModule);
	}

	void EngineLoader::ReadModule(Module* pModule)
	{
		const ModuleMetaData& metaData = pModule->GetMetaData();
		const ModuleType moduleType = metaData.Type();

		/* FIXME: There is a better place for this */
		if (moduleType == ModuleType::MT_Window)
		{
			((WindowModule*)pModule)->SetMainWindowCreateInfo(m_DefaultWindow);
		}

		if (moduleType < ModuleType::MT_Loader)
		{
			m_pMainModules.push_back(pModule);
			return;
		}

		m_pOptionalModules.push_back(pModule);
	}

	void EngineLoader::LoadExtras()
	{
		for (size_t i = 0; i < m_pAllModules.size(); ++i)
		{
			const ModuleMetaData& meta = m_pAllModules[i]->GetMetaData();
			for (size_t i = 0; i < meta.NumExtras(); ++i)
			{
				const ModuleExtra& extra = meta.Extra(i);
				Module* pRequired = nullptr;
				if (!extra.m_Requires.empty())
				{
					const auto itor = std::find(m_LoadedModuleNames.begin(), m_LoadedModuleNames.end(), extra.m_Requires);
					if (itor == m_LoadedModuleNames.end())
					{
						std::stringstream stream;
						stream << "Skipping loading of module extra \"" << extra.m_File << "\", the required module \"" << extra.m_Requires << "\" is not loaded";
						m_EngineInfo.m_pDebug->LogInfo(stream.str());
						continue;
					}
					const size_t index = itor - m_LoadedModuleNames.begin();
					pRequired = m_pAllModules[index];
				}

				std::filesystem::path filePath = meta.Path().parent_path();
				filePath.append(extra.m_File);
				LoadExtra(extra.m_File, filePath, m_pAllModules[i], pRequired);
			}
		}
	}

	void EngineLoader::LoadExtra(const std::string& name, const std::filesystem::path& path, Module* pModule, Module* pRequiredModule)
	{
		std::stringstream debugStream;
		debugStream << "Loading module extra \"" << name << "\"...";
		m_EngineInfo.m_pDebug->LogInfo(debugStream.str());

		/* Load lib */
		std::filesystem::path dllPath = path;
		dllPath.replace_extension(".dll");
		if (!std::filesystem::exists(dllPath))
		{
			debugStream.clear();
			debugStream << "Failed to load module extra: " << name << ": The module was not found!";
			m_EngineInfo.m_pDebug->LogError(debugStream.str());
			return;
		}

		HMODULE lib = LoadLibrary(dllPath.wstring().c_str());
		if (lib == NULL)
		{
			debugStream.clear();
			debugStream << "Failed to load module extra: " << name << ": There was an error while loading the library!";
			m_EngineInfo.m_pDebug->LogError(debugStream.str());
			m_EngineInfo.m_pDebug->LogError(GetLastErrorAsString());
			return;
		}

		OnLoadExtraProc loadProc = (OnLoadExtraProc)GetProcAddress(lib, "OnLoadExtra");
		if (loadProc == NULL)
		{
			FreeLibrary(lib);
			m_EngineInfo.m_pDebug->LogError("Failed to load module extra: " + name + ": Missing OnLoadExtra function!");
			return;
		}

		if (!(loadProc)(dllPath.parent_path().string().data(), pModule, pRequiredModule))
		{
			FreeLibrary(lib);
			m_EngineInfo.m_pDebug->LogError("Failed to load module extra: " + name + ": OnLoadExtra return false!");
			return;
		}

		m_ExtraLibs.push_back(lib);
	}
}

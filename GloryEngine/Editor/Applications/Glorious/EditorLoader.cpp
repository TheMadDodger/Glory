#include "pch.h"
#include "EditorLoader.h"

#include <Debug.h>

namespace Glory
{
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

	typedef void(__cdecl* LoadBackendProc)(EditorCreateInfo&);
	typedef Editor::BaseEditorExtension*(__cdecl* LoadExtensionProc)();

	EditorLoader::EditorLoader()
	{
	}

	EditorLoader::~EditorLoader()
	{
	}

	Glory::EditorCreateInfo EditorLoader::LoadEditor(Engine* pEngine, EngineLoader& engineLoader)
	{
		Glory::EditorCreateInfo editorCreateInfo{};
		editorCreateInfo.pEngine = pEngine;

		for (size_t i = 0; i < engineLoader.ModuleCount(); i++)
		{
			const Module* pModule = engineLoader.GetModule(i);
			const ModuleMetaData& metaData = pModule->GetMetaData();
			LoadBackend(editorCreateInfo, metaData);
			LoadExtensions(pEngine, metaData);
		}

		LoadGlobalExtensionDependencies(pEngine);
		LoadGlobalExtensions(pEngine);

		editorCreateInfo.ExtensionsCount = static_cast<uint32_t>(m_pExtensions.size());
		editorCreateInfo.pExtensions = m_pExtensions.data();
		return editorCreateInfo;
	}

	void EditorLoader::Unload()
	{
		for (size_t i = 0; i < m_pExtensions.size(); i++)
		{
			delete m_pExtensions[i];
		}
		m_pExtensions.clear();

		for (size_t i = 0; i < m_Libs.size(); i++)
		{
			FreeLibrary(m_Libs[i]);
		}
		m_Libs.clear();
	}

	void EditorLoader::LoadModuleMetadata(Glory::EditorCreateInfo& editorCreateInfo, const std::string& name)
	{
		std::filesystem::path modulesPath = "./Modules";
		std::filesystem::path modulePath = modulesPath;
		std::filesystem::path metaDataPath = modulePath.append(name);
		metaDataPath.append("Module.yaml");

		YAML::Node node = YAML::LoadFile(metaDataPath.string());
		YAML::Node typeNode = node["Type"];
		YAML::Node editorNode = node["EditorBackend"];
		std::string type = typeNode.as<std::string>();
		std::string edtorDLLName = editorNode.as<std::string>();

		std::filesystem::path dllPath = modulePath.append("Editor").append("Backend").append(edtorDLLName).replace_extension(".dll");

		HMODULE lib = LoadLibrary(dllPath.wstring().c_str());
		if (lib == NULL)
		{
			editorCreateInfo.pEngine->GetDebug().LogFatalError("Failed to load editor backend for module: " + name + ": The dll was not found!");
			return;
		}

		LoadBackendProc loadProc = (LoadBackendProc)GetProcAddress(lib, "LoadBackend");
		if (loadProc == NULL)
		{
			FreeLibrary(lib);
			editorCreateInfo.pEngine->GetDebug().LogError("Failed to load editor backend for module: " + name + ": Missing LoadBackend function!");
			return;
		}

		(loadProc)(editorCreateInfo);
		m_Libs.push_back(lib);
	}

	void EditorLoader::LoadBackend(Glory::EditorCreateInfo& editorCreateInfo, const ModuleMetaData& metaData)
	{
		const std::string& name = metaData.Name();
		const std::filesystem::path& path = metaData.Path();
		const std::string& editorBackend = metaData.EditorBackend();

		if (editorBackend == "") return;

		std::filesystem::path modulePath = path.parent_path();
		std::filesystem::path dllPath = modulePath.append("Editor").append("Backend").append(editorBackend).replace_extension(".dll");
		LoadBackendDLL(dllPath, editorBackend, editorCreateInfo);
	}

	void EditorLoader::LoadExtensions(Engine* pEngine, const ModuleMetaData& metaData)
	{
		const std::vector<std::string>& extensionNames = metaData.EditorExtensions();
		std::filesystem::path extensionsPath = metaData.Path();
		extensionsPath = extensionsPath.parent_path().append("Editor").append("Extension");
		for (size_t i = 0; i < extensionNames.size(); i++)
		{
			std::filesystem::path path = extensionsPath;
			path.append(extensionNames[i]).replace_extension(".dll");
			LoadExtensionDLL(pEngine, path, extensionNames[i]);
		}
	}

	void EditorLoader::LoadBackendDLL(const std::filesystem::path& dllPath, const std::string& name, Glory::EditorCreateInfo& editorCreateInfo)
	{
		editorCreateInfo.pEngine->GetDebug().LogInfo("Loading editor backend: " + name + "...");
		HMODULE lib = LoadLibrary(dllPath.wstring().c_str());
		if (lib == NULL)
		{
			editorCreateInfo.pEngine->GetDebug().LogFatalError("Failed to load editor backend: " + name + ": The dll was not found!");
			return;
		}

		LoadBackendProc loadProc = (LoadBackendProc)GetProcAddress(lib, "LoadBackend");
		if (loadProc == NULL)
		{
			FreeLibrary(lib);
			editorCreateInfo.pEngine->GetDebug().LogError("Failed to load editor backend: " + name + ": Missing LoadBackend function!");
			return;
		}

		(loadProc)(editorCreateInfo);
		m_Libs.push_back(lib);
	}

	void EditorLoader::LoadExtensionDLL(Engine* pEngine, const std::filesystem::path& dllPath, const std::string& name)
	{
		pEngine->GetDebug().LogInfo("Loading editor extension: " + name + "...");
		if (!std::filesystem::exists(dllPath))
		{
			pEngine->GetDebug().LogError("Failed to load editor extension: " + name + ": The dll was not found!");
		}

		HMODULE lib = LoadLibrary(dllPath.wstring().c_str());
		if (lib == NULL)
		{
			pEngine->GetDebug().LogError("Failed to load editor extension: " + name + ": There was an error while loading the library!");
			pEngine->GetDebug().LogError(GetLastErrorAsString());
			return;
		}

		LoadExtensionProc loadProc = (LoadExtensionProc)GetProcAddress(lib, "LoadExtension");
		if (loadProc == NULL)
		{
			FreeLibrary(lib);
			pEngine->GetDebug().LogError("Failed to load editor extension: " + name + ": Missing LoadExtension function!");
			return;
		}

		SetContextProc contextProc = (SetContextProc)GetProcAddress(lib, "SetContext");

		Editor::BaseEditorExtension* pExtension = (loadProc)();
		m_Libs.push_back(lib);
		if (pExtension == nullptr) return;
		m_pExtensions.push_back(pExtension);
		pExtension->SetSetContextProc(contextProc);
	}

	void EditorLoader::LoadExtensionDependencyDLL(Engine* pEngine, const std::filesystem::path& dllPath, const std::string& name)
	{
		pEngine->GetDebug().LogInfo("Loading editor extension dependency: " + name + "...");
		HMODULE lib = LoadLibrary(dllPath.wstring().c_str());
		if (lib == NULL)
		{
			pEngine->GetDebug().LogError("Failed to load editor extension dependency: " + name + ": The dll was not found!");
			return;
		}
		m_Libs.push_back(lib);
	}

	void EditorLoader::LoadGlobalExtensionDependencies(Engine* pEngine)
	{
		const std::filesystem::path extensionsPath = "./Extensions/Dependencies";

		for (const auto& entry : std::filesystem::directory_iterator(extensionsPath))
		{
			/* Recursive extensions are not supported! */
			if (entry.is_directory()) continue;

			const std::filesystem::path file = entry.path();
			if (file.extension().compare(".dll") != 0) continue;
			const std::string name = file.filename().replace_extension("").string();
			LoadExtensionDependencyDLL(pEngine, file, name);
		}
	}

	void EditorLoader::LoadGlobalExtensions(Engine* pEngine)
	{
		const std::filesystem::path extensionsPath = "./Extensions";

		for (const auto& entry : std::filesystem::directory_iterator(extensionsPath))
		{
			/* Recursive extensions are not supported! */
			if (entry.is_directory()) continue;

			const std::filesystem::path file = entry.path();
			if (file.extension().compare(".dll") != 0) continue;
			const std::string name = file.filename().replace_extension("").string();
			LoadExtensionDLL(pEngine, file, name);
		}
	}
}

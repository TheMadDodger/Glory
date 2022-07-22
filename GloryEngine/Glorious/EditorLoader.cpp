#include "pch.h"
#include "EditorLoader.h"
#include "EditorWindowImpl.h"
#include "EditorRenderImpl.h"

namespace Glory
{
	typedef void(__cdecl* LoadBackendProc)(EditorCreateInfo&);
	typedef void(__cdecl* LoadExtensionProc)(std::vector<Editor::BaseEditorExtension*>&);

	EditorLoader::EditorLoader()
	{
	}

	EditorLoader::~EditorLoader()
	{
	}

	Glory::EditorCreateInfo EditorLoader::LoadEditor(Game& game, EngineLoader& engineLoader)
	{
		Glory::EditorCreateInfo editorCreateInfo{};
		for (size_t i = 0; i < engineLoader.ModuleCount(); i++)
		{
			const Module* pModule = engineLoader.GetModule(i);
			const ModuleMetaData& metaData = pModule->GetMetaData();
			LoadBackend(editorCreateInfo, metaData);
			LoadExtensions(metaData);
		}

		editorCreateInfo.ExtensionsCount = static_cast<uint32_t>(m_pExtensions.size());
		editorCreateInfo.pExtensions = m_pExtensions.data();
		return editorCreateInfo;
	}

	void EditorLoader::Unload()
	{
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
			Debug::LogFatalError("Failed to load editor backend for module: " + name + ": The dll was not found!");
			return;
		}

		LoadBackendProc loadProc = (LoadBackendProc)GetProcAddress(lib, "LoadBackend");
		if (loadProc == NULL)
		{
			FreeLibrary(lib);
			Debug::LogError("Failed to load editor backend for module: " + name + ": Missing LoadBackend function!");
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

	void EditorLoader::LoadExtensions(const ModuleMetaData& metaData)
	{
		const std::vector<std::string>& extensionNames = metaData.EditorExtensions();
		std::filesystem::path extensionsPath = metaData.Path();
		extensionsPath = extensionsPath.parent_path().append("Editor").append("Extension");
		for (size_t i = 0; i < extensionNames.size(); i++)
		{
			std::filesystem::path path = extensionsPath;
			path.append(extensionNames[i]).replace_extension(".dll");
			LoadExtensionDLL(path, extensionNames[i]);
		}
	}

	void EditorLoader::LoadBackendDLL(const std::filesystem::path& dllPath, const std::string& name, Glory::EditorCreateInfo& editorCreateInfo)
	{
		Debug::LogInfo("Loading editor backend: " + name + "...");
		HMODULE lib = LoadLibrary(dllPath.wstring().c_str());
		if (lib == NULL)
		{
			Debug::LogFatalError("Failed to load editor backend: " + name + ": The dll was not found!");
			return;
		}

		LoadBackendProc loadProc = (LoadBackendProc)GetProcAddress(lib, "LoadBackend");
		if (loadProc == NULL)
		{
			FreeLibrary(lib);
			Debug::LogError("Failed to load editor backend: " + name + ": Missing LoadBackend function!");
			return;
		}

		(loadProc)(editorCreateInfo);
		m_Libs.push_back(lib);
	}

	void EditorLoader::LoadExtensionDLL(const std::filesystem::path& dllPath, const std::string& name)
	{
		Debug::LogInfo("Loading editor extension: " + name + "...");
		HMODULE lib = LoadLibrary(dllPath.wstring().c_str());
		if (lib == NULL)
		{
			Debug::LogFatalError("Failed to load editor extension: " + name + ": The dll was not found!");
			return;
		}

		LoadExtensionProc loadProc = (LoadExtensionProc)GetProcAddress(lib, "LoadExtension");
		if (loadProc == NULL)
		{
			FreeLibrary(lib);
			Debug::LogError("Failed to load editor extension: " + name + ": Missing LoadExtension function!");
			return;
		}

		(loadProc)(m_pExtensions);
		m_Libs.push_back(lib);
	}
}

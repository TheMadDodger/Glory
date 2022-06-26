#include "pch.h"
#include "EditorLoader.h"
#include "EditorWindowImpl.h"
#include "EditorRenderImpl.h"

namespace Glory
{
	typedef void(__cdecl* LoadBackendProc)(EditorCreateInfo&);

	EditorLoader::EditorLoader()
	{
	}

	EditorLoader::~EditorLoader()
	{
	}

	Glory::EditorCreateInfo EditorLoader::LoadEditor(Game& game, EngineLoader& engineLoader)
	{
		const std::string& windowModule = engineLoader.GetSetModule("Window");
		const std::string& graphicsModule = engineLoader.GetSetModule("Graphics");

		Glory::EditorCreateInfo editorCreateInfo{};
		LoadModuleMetadata(editorCreateInfo, windowModule);
		LoadModuleMetadata(editorCreateInfo, graphicsModule);
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

		
		std::filesystem::path dllPath = modulePath.append("EditorBackend").append(edtorDLLName).replace_extension(".dll");

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
}

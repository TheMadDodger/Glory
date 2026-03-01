#pragma once
#include <EngineLoader.h>
#include <EditorApplication.h>
#include <Package.h>

namespace Glory
{
	class IEngine;

	class EditorLoader
	{
	public:
		EditorLoader();
		virtual ~EditorLoader();

		Glory::EditorCreateInfo LoadEditor(IEngine* pEngine, EngineLoader& engineLoader);
		void Unload();

	private:
		void LoadModuleMetadata(Glory::EditorCreateInfo& editorCreateInfo, const std::string& name);
		void LoadBackend(Glory::EditorCreateInfo& editorCreateInfo, const ModuleMetaData& metaData);
		void LoadExtensions(IEngine* pEngine, const ModuleMetaData& metaData);

		void LoadBackendDLL(const std::filesystem::path& dllPath, const std::string& name, Glory::EditorCreateInfo& editorCreateInfo);
		void LoadExtensionDLL(IEngine* pEngine, const std::filesystem::path& dllPath, const std::string& name);
		void LoadExtensionDependencyDLL(IEngine* pEngine, const std::filesystem::path& dllPath, const std::string& name);

		void LoadGlobalExtensionDependencies(IEngine* pEngine);
		void LoadGlobalExtensions(IEngine* pEngine);

	private:
		std::vector<HMODULE> m_Libs;
		std::vector<Editor::BaseEditorExtension*> m_pExtensions;
	};
}

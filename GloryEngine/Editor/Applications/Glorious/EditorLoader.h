#pragma once
#include <EngineLoader.h>
#include <EditorApplication.h>

namespace Glory
{
	class EditorLoader
	{
	public:
		EditorLoader();
		virtual ~EditorLoader();

		Glory::EditorCreateInfo LoadEditor(Engine* pEngine, EngineLoader& engineLoader);
		void Unload();

	private:
		void LoadModuleMetadata(Glory::EditorCreateInfo& editorCreateInfo, const std::string& name);
		void LoadBackend(Glory::EditorCreateInfo& editorCreateInfo, const ModuleMetaData& metaData);
		void LoadExtensions(Engine* pEngine, const ModuleMetaData& metaData);

		void LoadBackendDLL(const std::filesystem::path& dllPath, const std::string& name, Glory::EditorCreateInfo& editorCreateInfo);
		void LoadExtensionDLL(Engine* pEngine, const std::filesystem::path& dllPath, const std::string& name);
		void LoadExtensionDependencyDLL(Engine* pEngine, const std::filesystem::path& dllPath, const std::string& name);

		void LoadGlobalExtensionDependencies(Engine* pEngine);
		void LoadGlobalExtensions(Engine* pEngine);

	private:
		std::vector<HMODULE> m_Libs;
		std::vector<Editor::BaseEditorExtension*> m_pExtensions;
	};
}

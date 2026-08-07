#pragma once
#include "EditorApplication.h"
#include "Package.h"

#include <EngineLoader.h>

namespace Glory
{
	class IEngine;

	class EditorLoader
	{
	public:
		GLORY_EDITOR_API EditorLoader();
		GLORY_EDITOR_API virtual ~EditorLoader();

		GLORY_EDITOR_API Glory::EditorCreateInfo LoadEditor(IEngine* pEngine, EngineLoader& engineLoader);
		GLORY_EDITOR_API void Unload();

		GLORY_EDITOR_API Editor::BaseEditorExtension* GetExtension(const std::string& name);

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
		std::vector<std::string> m_ExtensionNames;
		std::vector<Editor::BaseEditorExtension*> m_pExtensions;
	};
}

#pragma once
#include "EditorPlatform.h"
#include "MainEditor.h"
#include "ProjectSpace.h"
#include "EditorCreateInfo.h"
#include "EditorPlayer.h"

#include <Version.h>
#include <memory>
#include <iosfwd>

namespace efsw
{
	class FileWatcher;
}

namespace Glory
{
	class Engine;

namespace Editor
{

	enum class EditorMode
	{
		M_Edit,
		M_EnteringPlay,
		M_Play,
		M_ExitingPlay,
	};

	class EditorAssetManager;
	class EditorSceneManager;
	class EditorShaderProcessor;
	class EditorResourceManager;
	class EditorPipelineManager;
	class EditorMaterialManager;

	class EditorApplication
	{
	public:
		GLORY_EDITOR_API EditorApplication(const EditorCreateInfo& createInfo);
		virtual GLORY_EDITOR_API ~EditorApplication();

		GLORY_EDITOR_API void Initialize();

		GLORY_EDITOR_API void Destroy();
		GLORY_EDITOR_API void Run();

		GLORY_EDITOR_API EditorPlatform& GetEditorPlatform();
		GLORY_EDITOR_API MainEditor& GetMainEditor();
		GLORY_EDITOR_API efsw::FileWatcher& FileWatch();

		GLORY_EDITOR_API const EditorMode& CurrentMode();
		GLORY_EDITOR_API void TogglePlay();
		GLORY_EDITOR_API void StartPlay();
		GLORY_EDITOR_API void StopPlay();
		GLORY_EDITOR_API void TogglePause();
		GLORY_EDITOR_API void TickFrame();
		GLORY_EDITOR_API bool IsPaused();
		GLORY_EDITOR_API void Quit();
		GLORY_EDITOR_API void TryToQuit();
		GLORY_EDITOR_API Glory::Engine* GetEngine();
		GLORY_EDITOR_API EditorAssetManager& GetAssetManager();
		GLORY_EDITOR_API EditorSceneManager& GetSceneManager();
		GLORY_EDITOR_API EditorShaderProcessor& GetShaderProcessor();
		GLORY_EDITOR_API EditorResourceManager& GetResourceManager();
		GLORY_EDITOR_API EditorPipelineManager& GetPipelineManager();
		GLORY_EDITOR_API EditorMaterialManager& GetMaterialManager();

		GLORY_EDITOR_API void OnFileDragAndDrop(std::vector<std::string_view>& paths);
		GLORY_EDITOR_API static EditorApplication* GetInstance();

		GLORY_EDITOR_API void OnBeginPackage(const std::filesystem::path& path);
		GLORY_EDITOR_API void OnGenerateConfigExec(std::ofstream& stream);
		GLORY_EDITOR_API void OnEndPackage(const std::filesystem::path& path);
		GLORY_EDITOR_API void UpdateExtensions();

		static const Version Version;

	private:
		void RenderEditor();
		void InitializePlatform();
		void InitializeExtensions();

		static void VersionCheck(const Glory::Version& latestVersion);

	private:
		Glory::Engine* m_pEngine;

		EditorPlatform m_Platform;
		MainEditor m_MainEditor;
		EditorPlayer m_Player;
		std::unique_ptr<EditorAssetManager> m_AssetManager;
		std::unique_ptr<EditorSceneManager> m_SceneManager;
		std::unique_ptr<EditorShaderProcessor> m_ShaderProcessor;
		std::unique_ptr<EditorResourceManager> m_ResourceManager;
		std::unique_ptr<EditorPipelineManager> m_PipelineManager;
		std::unique_ptr<EditorMaterialManager> m_MaterialManager;
		efsw::FileWatcher* m_pFileWatcher;

		std::vector<BaseEditorExtension*> m_pExtensions;
		EditorMode m_Mode = EditorMode::M_Edit;
		bool m_Running = false;
	};
}
}

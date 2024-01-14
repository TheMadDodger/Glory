#pragma once
#include "EditorPlatform.h"
#include "MainEditor.h"
#include "ProjectSpace.h"
#include "EditorShaderProcessor.h"
#include "EditorCreateInfo.h"
#include "EditorPlayer.h"

#include <Version.h>

namespace efsw
{
	class FileWatcher;
}

namespace Glory::Editor
{
	enum class EditorMode
	{
		M_Edit,
		M_EnteringPlay,
		M_Play,
		M_ExitingPlay,
	};

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
		GLORY_EDITOR_API Engine* GetEngine();

		GLORY_EDITOR_API void OnFileDragAndDrop(std::string_view path);
		GLORY_EDITOR_API static EditorApplication* GetInstance();

		static const Version Version;

	private:
		void RenderEditor();
		void InitializePlatform();
		void InitializeExtensions();

		static std::string AssetPathOverrider();
		static std::string SettingsPathOverrider();

		static void VersionCheck(const Glory::Version& latestVersion);

	private:
		Engine* m_pEngine;

		EditorPlatform m_Platform;
		MainEditor m_MainEditor;
		EditorPlayer m_Player;
		EditorShaderProcessor m_ShaderProcessor;
		efsw::FileWatcher* m_pFileWatcher;

		std::vector<BaseEditorExtension*> m_pExtensions;
		EditorMode m_Mode = EditorMode::M_Edit;
		bool m_Running = false;
	};
}

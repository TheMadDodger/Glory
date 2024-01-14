#include "EditorApplication.h"
#include "QuitPopup.h"
#include "EditorAssetDatabase.h"
#include "EditorAssetsWatcher.h"
#include "EntityEditor.h"

#include <GraphicsThread.h>
#include <imgui.h>
#include <Console.h>
#include <implot.h>
#include <ProjectSpace.h>
#include <EditorSceneManager.h>
#include <Version.h>

#include <GloryAPI.h>

#include <SceneManager.h>

namespace Glory::Editor
{
	EditorApplication* Instance = nullptr;

	const Version EditorApplication::Version = Version::Parse(GloryEditorVersion);

	EditorAssetsWatcher* AssetsWatcher = nullptr;

	EditorApplication::EditorApplication(const EditorCreateInfo& createInfo):
		m_pEngine(createInfo.pEngine),
		m_Platform(createInfo.pWindowImpl, createInfo.pRenderImpl),
		m_pFileWatcher(new efsw::FileWatcher())
	{
		// Copy the optional modules into the optional modules vector
		if (createInfo.ExtensionsCount > 0 && createInfo.pExtensions != nullptr)
		{
			m_pExtensions.resize(createInfo.ExtensionsCount);
			for (size_t i = 0; i < createInfo.ExtensionsCount; i++)
			{
				m_pExtensions[i] = createInfo.pExtensions[i];
			}
		}

		Instance = this;
	}

	EditorApplication::~EditorApplication()
	{
		delete m_pFileWatcher;
		m_pFileWatcher = nullptr;
	}

	void EditorApplication::Initialize()
	{
		m_pEngine->Initialize();

		EditorAssetDatabase::Initialize();

		InitializePlatform();

		GloryAPI::Initialize();

		m_pFileWatcher->watch();

		AssetsWatcher = new EditorAssetsWatcher();
	}

	void EditorApplication::InitializeExtensions()
	{
		for (size_t i = 0; i < m_pExtensions.size(); i++)
		{
			m_pExtensions[i]->SetCurrentContext();
			m_pExtensions[i]->Initialize();
		}
	}

	void EditorApplication::Destroy()
	{
		EditorAssetDatabase::Cleanup();
		m_MainEditor.Destroy();
		m_Platform.Destroy();
		m_ShaderProcessor.Stop();

		GloryAPI::Cleanup();

		delete AssetsWatcher;
		AssetsWatcher = nullptr;

		DestroyAllEditableEntities();
	}

	void EditorApplication::Run()
	{
		GloryAPI::FetchEditorVersion(VersionCheck);

		/* @fixme There is a better place for this */
		m_pEngine->GetSceneManager()->ComponentTypesInstance();

		m_pEngine->StartThreads();
		m_Platform.SetState(Idle);
		m_ShaderProcessor.Start();

		m_Running = true;
		if (m_Platform.m_Windowless)
		{
			while (m_Running)
			{
				/* Ensure filewatch is still watching */
				m_pFileWatcher->watch();

				EditorAssetsWatcher::RunCallbacks();

				// Start a frame
				m_pEngine->GameThreadFrameStart();

				// Update console
				m_pEngine->GetConsole().Update();

				// Update asset database
				EditorAssetDatabase::Update();

				m_Player.Tick(m_pEngine);

				// Update engine (this also does the render loop)
				m_pEngine->ModulesLoop(&m_Player);

				// End the current frame
				m_pEngine->GameThreadFrameEnd();
			}
			return;
		}

		while (m_Running)
		{
			/* Ensure filewatch is still watching */
			m_pFileWatcher->watch();

			EditorAssetsWatcher::RunCallbacks();

			/* We must wait for graphics to initialize */
			if (!m_pEngine->GetGraphicsThread()->IsInitialized()) continue;

			// Start a frame
			m_pEngine->GameThreadFrameStart();
			// Update console
			m_pEngine->GetConsole().Update();

			// Poll window events
			if (m_Platform.PollEvents()) TryToQuit();

			// Update editor
			m_MainEditor.Update();

			// Update asset database
			EditorAssetDatabase::Update();

			m_Player.Tick(m_pEngine);

			// Update engine (this also does the render loop)
			m_pEngine->ModulesLoop(&m_Player);

			// End the current frame
			m_pEngine->GameThreadFrameEnd();

			// We need to wait for the frame to start its rendering
			m_Platform.Wait(Begin);
			// Render the editor (imgui calls)
			RenderEditor();
			/* Run API callbacks */
			GloryAPI::RunRequests();
			// Now we notify the editor platform it can perform rendering
			m_Platform.SetState(Idle);
			// Wait for the end of rendering
			m_Platform.Wait(End);
			// Sync
			m_Platform.SetState(Idle);
		}
	}

	void EditorApplication::Quit()
	{
		m_Running = false;
	}

	EditorPlatform& EditorApplication::GetEditorPlatform()
	{
		return m_Platform;
	}

	MainEditor& EditorApplication::GetMainEditor()
	{
		return m_MainEditor;
	}

	efsw::FileWatcher& EditorApplication::FileWatch()
	{
		return *m_pFileWatcher;
	}

	const EditorMode& EditorApplication::CurrentMode()
	{
		return m_Mode;
	}

	void EditorApplication::TogglePlay()
	{
		switch (m_Mode)
		{
		case Glory::Editor::EditorMode::M_Edit:
			StartPlay();
			break;
		case Glory::Editor::EditorMode::M_Play:
			StopPlay();
			break;
		}
	}

	void EditorApplication::StartPlay()
	{
		if (m_Mode != EditorMode::M_Edit) return;
		m_pEngine->GetDebug().LogInfo("Entering play mode");
		m_Mode = EditorMode::M_EnteringPlay;
		m_Player.Start();
		m_Mode = EditorMode::M_Play;
	}

	void EditorApplication::StopPlay()
	{
		if (m_Mode != EditorMode::M_Play) return;
		m_pEngine->GetDebug().LogInfo("Entering edit mode");
		m_Mode = EditorMode::M_ExitingPlay;
		m_Player.Stop();
		m_Mode = EditorMode::M_Edit;
	}

	void EditorApplication::TogglePause()
	{
		m_Player.TogglePauze();
	}

	void EditorApplication::TickFrame()
	{
		if (m_Mode != EditorMode::M_Play) return;
		m_Player.TickFrame();
	}

	bool EditorApplication::IsPaused()
	{
		return m_Player.m_IsPaused;
	}

	EditorApplication* EditorApplication::GetInstance()
	{
		return Instance;
	}

	void EditorApplication::RenderEditor()
	{
		m_MainEditor.PaintEditor();
		//ImGui::ShowDemoWindow();
		//ImPlot::ShowDemoWindow();
	}

	void EditorApplication::InitializePlatform()
	{
		m_Platform.Initialize(m_pEngine);
		m_MainEditor.Initialize();

		InitializeExtensions();

		m_pEngine->GetDebug().LogInfo("Initialized editor platform");
	}

	std::string EditorApplication::AssetPathOverrider()
	{
		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		if (pProject == nullptr) return std::string("./Assets");
		std::filesystem::path path = pProject->RootPath();
		path.append("Assets");
		return path.string();
	}

	std::string EditorApplication::SettingsPathOverrider()
	{
		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		if (pProject == nullptr) return std::string("./");
		std::filesystem::path path = pProject->SettingsPath();
		return path.string();
	}

	void EditorApplication::VersionCheck(const Glory::Version& latestVersion)
	{
		if (!latestVersion.IsValid()) return;
		if (Version::Compare(latestVersion, Version) == 1)
			MainEditor::VersionOutdated(latestVersion);
	}

	Engine* EditorApplication::GetEngine()
	{
		return m_pEngine;
	}

	void EditorApplication::TryToQuit()
	{
		if (ProjectSpace::HasUnsavedChanges())
		{
			QuitPopup::Open("You have unsaved changes to your project.\nAre you sure you want to quit?\nAll changes will be lost!");
			/* Show popup */
			return;
		}

		if (EditorSceneManager::HasUnsavedChanges())
		{
			/* Show popup */
			QuitPopup::Open("You have unsaved changes to an opened scene.\nAre you sure you want to quit?\nAll changes will be lost!");
			return;
		}

		Quit();
	}

	void EditorApplication::OnFileDragAndDrop(std::string_view path)
	{
		std::stringstream stream;
		stream << "Drag and drop file received at " << path;
		m_pEngine->GetDebug().LogInfo(stream.str());
		m_MainEditor.OnFileDragAndDrop(path);
	}
}

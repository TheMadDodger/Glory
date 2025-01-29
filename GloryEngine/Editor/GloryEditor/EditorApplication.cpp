#include "EditorApplication.h"
#include "QuitPopup.h"
#include "EditorAssetDatabase.h"
#include "EditorAssetsWatcher.h"
#include "EntityEditor.h"
#include "EditorResourceManager.h"
#include "EditorSceneManager.h"
#include "ProjectSpace.h"
#include "EditorAssetManager.h"
#include "EditorShaderProcessor.h"
#include "EditorPipelineManager.h"
#include "EditorMaterialManager.h"
#include "AssetCompiler.h"
#include "FileBrowser.h"
#include "ProjectSettings.h"

#include <imgui.h>
#include <Console.h>
#include <implot.h>
#include <Version.h>

#include <GloryAPI.h>

#include <SceneManager.h>
#include <WindowModule.h>
#include <ThreadManager.h>

namespace Glory::Editor
{
	EditorApplication* Instance = nullptr;

	const Version EditorApplication::Version = Version::Parse(GloryEditorVersion);

	EditorAssetsWatcher* AssetsWatcher = nullptr;

	std::mutex m_StartupMutex;
	std::string m_StartupStatus = "";

	EditorApplication::EditorApplication(const EditorCreateInfo& createInfo):
		m_pEngine(createInfo.pEngine),
		m_Platform(createInfo.pWindowImpl, createInfo.pRenderImpl),
		m_AssetManager(new EditorAssetManager(this)),
		m_SceneManager(new EditorSceneManager(this)),
		m_ShaderProcessor(new EditorShaderProcessor(this)),
		m_ResourceManager(new EditorResourceManager(createInfo.pEngine)),
		m_PipelineManager(new EditorPipelineManager(createInfo.pEngine)),
		m_MaterialManager(new EditorMaterialManager(createInfo.pEngine)),
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

		m_pEngine->SetAssetManager(m_AssetManager.get());
		m_pEngine->SetSceneManager(m_SceneManager.get());
		m_pEngine->SetShaderManager(m_ShaderProcessor.get());
		m_pEngine->SetMaterialManager(m_MaterialManager.get());
		m_pEngine->SetPipelineManager(m_PipelineManager.get());

		Instance = this;
	}

	EditorApplication::~EditorApplication()
	{
		Destroy();

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
		m_PipelineManager->Initialize();
		m_MaterialManager->Initialize();
		m_ShaderProcessor->Start();

		m_Running = false;
		m_IsStarting = false;
	}

	void EditorApplication::InitializeExtensions()
	{
		for (size_t i = 0; i < m_pExtensions.size(); i++)
		{
			m_pExtensions[i]->SetCurrentContext();
			m_pExtensions[i]->Initialize();
		}
	}

	void EditorApplication::RenderStartup()
	{
		int width, height;
		m_pEngine->GetMainModule<WindowModule>()->GetMainWindow()->GetDrawableSize(&width, &height);

		ImGui::SetNextWindowPos({ width/2.0f, height/2.0f }, 0, {.5f, .5f});
		ImGui::OpenPopup("Welcome");
		ImGui::BeginPopupModal("Welcome", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		std::scoped_lock lock(m_StartupMutex);
		ImGui::TextUnformatted("Getting things ready for you.");
		ImGui::TextUnformatted(m_StartupStatus.data());
		ImGui::EndPopup();
	}

	void EditorApplication::Start(const std::string& projectPath)
	{
		ProjectSpace::OpenProject(projectPath);

		m_IsStarting = true;
		ThreadManager::Run([this] {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			RunStartup();
		});

		while (m_IsStarting)
		{
			/* Start a frame */
			if (!m_Platform.m_Windowless)
				m_pEngine->BeginFrame();

			/* Update console */
			m_pEngine->GetConsole().Update();

			/* Poll window events */
			if (m_Platform.PollEvents()) TryToQuit();

			/* Update editor */
			EditorWindow::UpdateWindows();

			if (!m_Platform.m_Windowless)
			{
				/* End the current frame */
				m_pEngine->EndFrame();

				/* Begin an ImGui frame */
				m_Platform.BeginFrame();
				/* Paint the editor(imgui calls) */
				RenderEditor();

				/* Show startup popup */
				RenderStartup();

				/* Render the ImGui frame */
				m_Platform.EndFrame();
			}
		}

		FileBrowser::LoadProject();
		ProjectSettings::Load(ProjectSpace::GetOpenProject());
	}

	void EditorApplication::Destroy()
	{
		EditorAssetDatabase::Cleanup();
		m_MainEditor.Destroy();
		m_Platform.Destroy();
		m_ShaderProcessor->Stop();
		m_PipelineManager->Cleanup();
		m_MaterialManager->Cleanup();

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

		m_Running = true;
		if (m_Platform.m_Windowless)
		{
			while (m_Running)
			{
				/* Ensure filewatch is still watching */
				m_pFileWatcher->watch();

				EditorAssetsWatcher::RunCallbacks();

				/* Run asset callbacks */
				m_AssetManager->RunCallbacks();

				/* Run callbacks for compiled shaders */
				m_ShaderProcessor->RunCallbacks();

				// Start a frame
				m_pEngine->BeginFrame();

				// Update console
				m_pEngine->GetConsole().Update();

				// Update asset database
				EditorAssetDatabase::Update();

				/* Update editor extensions */
				UpdateExtensions();

				m_Player.Tick(m_pEngine);

				// Update engine (this also does the render loop)
				m_pEngine->ModulesLoop(&m_Player);

				// End the current frame
				m_pEngine->EndFrame();
			}
			return;
		}

		while (m_Running)
		{
			/* Ensure filewatch is still watching */
			m_pFileWatcher->watch();

			EditorAssetsWatcher::RunCallbacks();

			/* Run asset callbacks */
			m_AssetManager->RunCallbacks();

			/* Run callbacks for compiled shaders */
			m_ShaderProcessor->RunCallbacks();

			// Start a frame
			m_pEngine->BeginFrame();

			// Update console
			m_pEngine->GetConsole().Update();

			// Poll window events
			if (m_Platform.PollEvents()) TryToQuit();

			// Update editor
			m_MainEditor.Update();

			// Update asset database
			EditorAssetDatabase::Update();

			/* Update editor extensions */
			UpdateExtensions();

			/* Tick the player */
			m_Player.Tick(m_pEngine);

			// Update engine (this also does the render loop)
			m_pEngine->ModulesLoop(&m_Player);

			/* End the current frame */
			m_pEngine->EndFrame();

			/* Run API callbacks */
			GloryAPI::RunRequests();

			/* Begin an ImGui frame */
			m_Platform.BeginFrame();
			/* Paint the editor(imgui calls) */
			RenderEditor();
			/* Render the ImGui frame */
			m_Platform.EndFrame();
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

	void EditorApplication::OnBeginPackage(const std::filesystem::path& path)
	{
		for (size_t i = 0; i < m_pExtensions.size(); i++)
		{
			m_pExtensions[i]->OnBeginPackage(path);
		}
	}

	void EditorApplication::OnEndPackage(const std::filesystem::path& path)
	{
		for (size_t i = 0; i < m_pExtensions.size(); i++)
		{
			m_pExtensions[i]->OnEndPackage(path);
		}
	}

	void EditorApplication::UpdateExtensions()
	{
		for (size_t i = 0; i < m_pExtensions.size(); i++)
		{
			m_pExtensions[i]->Update();
		}
	}

	void EditorApplication::OnGenerateConfigExec(std::ofstream& stream)
	{
		for (size_t i = 0; i < m_pExtensions.size(); i++)
		{
			m_pExtensions[i]->OnGenerateConfigExec(stream);
		}
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

	void EditorApplication::VersionCheck(const Glory::Version& latestVersion)
	{
		if (!latestVersion.IsValid()) return;
		if (Version::Compare(latestVersion, Version) == 1)
			MainEditor::VersionOutdated(latestVersion);
	}

	void EditorApplication::RunStartup()
	{
		std::unique_lock lock(m_StartupMutex);
		m_StartupStatus = "Importing Module Assets...";
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		EditorAssetDatabase::ImportModuleAssets();

		lock.lock();
		m_StartupStatus = "Compiling Asset Database...";
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		AssetCompiler::CompileAssetDatabase();

		lock.lock();
		m_StartupStatus = "Compiling Pipelines...";
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		AssetCompiler::CompilePipelines();
		EditorAssetDatabase::Update();

		lock.lock();
		m_StartupStatus = "Waiting for shader compilation to finish...";
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		EditorShaderProcessor::WaitIdle();
		EditorApplication::GetInstance()->GetShaderProcessor().RunCallbacks();

		lock.lock();
		m_StartupStatus = "Compiling new assets...";
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		AssetCompiler::CompileNewAssets();

		m_IsStarting = false;
	}

	Engine* EditorApplication::GetEngine()
	{
		return m_pEngine;
	}

	EditorAssetManager& EditorApplication::GetAssetManager()
	{
		return *m_AssetManager;
	}

	EditorSceneManager& EditorApplication::GetSceneManager()
	{
		return *m_SceneManager;
	}

	EditorShaderProcessor& EditorApplication::GetShaderProcessor()
	{
		return *m_ShaderProcessor;
	}

	EditorResourceManager& EditorApplication::GetResourceManager()
	{
		return *m_ResourceManager;
	}

	EditorPipelineManager& EditorApplication::GetPipelineManager()
	{
		return *m_PipelineManager;
	}

	EditorMaterialManager& EditorApplication::GetMaterialManager()
	{
		return *m_MaterialManager;
	}

	void EditorApplication::TryToQuit()
	{
		if (ProjectSpace::HasUnsavedChanges())
		{
			QuitPopup::Open("You have unsaved changes to your project.\nAre you sure you want to quit?\nAll changes will be lost!");
			/* Show popup */
			return;
		}

		if (m_SceneManager->HasUnsavedChanges())
		{
			/* Show popup */
			QuitPopup::Open("You have unsaved changes to an opened scene.\nAre you sure you want to quit?\nAll changes will be lost!");
			return;
		}

		Quit();
	}

	void EditorApplication::OnFileDragAndDrop(std::vector<std::string_view>& paths)
	{
		m_MainEditor.OnFileDragAndDrop(paths);
	}
}

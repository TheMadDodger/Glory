#include "EditorApplication.h"
#include "QuitPopup.h"
#include "EditorAssetDatabase.h"
#include "EditorAssetsWatcher.h"

#include <GraphicsThread.h>
#include <imgui.h>
#include <Console.h>
#include <implot.h>
#include <ProjectSpace.h>
#include <EditorSceneManager.h>
#include <Version.h>

#include <GloryAPI.h>

namespace Glory::Editor
{
	EditorApplication* EditorApplication::m_pEditorInstance = nullptr;
	const Version EditorApplication::Version = Version::Parse(GloryEditorVersion);
	EditorMode EditorApplication::m_Mode = EditorMode::M_Edit;
	bool EditorApplication::m_Running = false;

	EditorAssetsWatcher* AssetsWatcher = nullptr;

	EditorApplication::EditorApplication(const EditorCreateInfo& createInfo)
		: m_pMainEditor(nullptr), m_pPlatform(nullptr), m_pTempWindowImpl(createInfo.pWindowImpl),
		m_pTempRenderImpl(createInfo.pRenderImpl), m_pShaderProcessor(nullptr), m_pPlayer(nullptr),
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

		GloryContext::SetContext(createInfo.pContext);
	}

	EditorApplication::~EditorApplication()
	{
		delete m_pMainEditor;
		m_pMainEditor = nullptr;

		delete m_pPlatform;
		m_pPlatform = nullptr;

		delete m_pFileWatcher;
		m_pFileWatcher = nullptr;
	}

	void EditorApplication::Initialize(Game& game)
	{
		m_pEditorInstance = this;

		game.OverrideAssetPathFunc(EditorApplication::AssetPathOverrider);
		game.OverrideSettingsPathFunc(EditorApplication::SettingsPathOverrider);

		m_pShaderProcessor = new EditorShaderProcessor();
		EditorAssetDatabase::Initialize();

		if (!m_pTempWindowImpl || !m_pTempRenderImpl) return;
		m_pPlatform = new EditorPlatform(m_pTempWindowImpl, m_pTempRenderImpl);
		m_pTempWindowImpl->m_pEditorPlatform = m_pPlatform;
		m_pTempRenderImpl->m_pEditorPlatform = m_pPlatform;
		m_pTempWindowImpl = nullptr;
		m_pTempRenderImpl = nullptr;

		InitializePlatform(game);

		GloryAPI::Initialize();

		m_pFileWatcher->watch();

		AssetsWatcher = new EditorAssetsWatcher();
	}

	void EditorApplication::InitializeExtensions()
	{
		for (size_t i = 0; i < m_pExtensions.size(); i++)
		{
			m_pExtensions[i]->SetCurrentContext();
			m_pExtensions[i]->RegisterEditors();
		}
	}

	void EditorApplication::Destroy()
	{
		EditorAssetDatabase::Cleanup();
		if(m_pMainEditor) m_pMainEditor->Destroy();
		if(m_pPlatform) m_pPlatform->Destroy();
		m_pShaderProcessor->Stop();
		delete m_pShaderProcessor;
		m_pShaderProcessor = nullptr;

		if(m_pPlayer) delete m_pPlayer;
		m_pPlayer = nullptr;

		GloryAPI::Cleanup();

		delete AssetsWatcher;
		AssetsWatcher = nullptr;
	}

	void EditorApplication::Run(Game& game)
	{
		GloryAPI::FetchEditorVersion(VersionCheck);

		game.GetEngine()->StartThreads();
		if(m_pPlatform) m_pPlatform->SetState(Idle);
		m_pShaderProcessor->Start();

		m_Running = true;
		if (!m_pPlatform)
		{
			while (m_Running)
			{
				/* Ensure filewatch is still watching */
				m_pFileWatcher->watch();

				EditorAssetsWatcher::RunCallbacks();

				// Start a frame
				game.GetEngine()->GameThreadFrameStart();

				// Update console
				Console::Update();

				// Update asset database
				EditorAssetDatabase::Update();

				// Update engine (this also does the render loop)
				game.GetEngine()->ModulesLoop(m_pPlayer);

				// End the current frame
				game.GetEngine()->GameThreadFrameEnd();
			}
			return;
		}

		while (m_Running)
		{
			/* Ensure filewatch is still watching */
			m_pFileWatcher->watch();

			EditorAssetsWatcher::RunCallbacks();

			/* We must wait for graphics to initialize */
			if (!game.GetEngine()->GetGraphicsThread()->IsInitialized()) continue;

			// Start a frame
			game.GetEngine()->GameThreadFrameStart();
			// Update console
			Console::Update();

			// Poll window events
			if (m_pPlatform->PollEvents()) TryToQuit();

			// Update editor
			m_pMainEditor->Update();

			// Update asset database
			EditorAssetDatabase::Update();

			// Update engine (this also does the render loop)
			game.GetEngine()->ModulesLoop(m_pPlayer);

			// End the current frame
			game.GetEngine()->GameThreadFrameEnd();

			// We need to wait for the frame to start its rendering
			m_pPlatform->Wait(Begin);
			// Render the editor (imgui calls)
			RenderEditor();
			/* Run API callbacks */
			GloryAPI::RunRequests();
			// Now we notify the editor platform it can perform rendering
			m_pPlatform->SetState(Idle);
			// Wait for the end of rendering
			m_pPlatform->Wait(End);
			// Sync
			m_pPlatform->SetState(Idle);
		}
	}

	void EditorApplication::Quit()
	{
		m_Running = false;
	}

	void EditorApplication::SetWindowImpl(EditorWindowImpl* pWindowImpl)
	{
		m_pTempWindowImpl = pWindowImpl;
	}

	void EditorApplication::SetRendererImpl(EditorRenderImpl* pRendererImpl)
	{
		m_pTempRenderImpl = pRendererImpl;
	}

	EditorPlatform* EditorApplication::GetEditorPlatform()
	{
		return m_pPlatform;
	}

	MainEditor* EditorApplication::GetMainEditor()
	{
		return m_pMainEditor;
	}

	efsw::FileWatcher* EditorApplication::FileWatch()
	{
		return m_pFileWatcher;
	}

	EditorApplication* EditorApplication::GetInstance()
	{
		return m_pEditorInstance;
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
		Debug::LogInfo("Entering play mode");
		m_Mode = EditorMode::M_EnteringPlay;
		m_pEditorInstance->m_pPlayer->Start();
		m_Mode = EditorMode::M_Play;
	}

	void EditorApplication::StopPlay()
	{
		if (m_Mode != EditorMode::M_Play) return;
		Debug::LogInfo("Entering edit mode");
		m_Mode = EditorMode::M_ExitingPlay;
		m_pEditorInstance->m_pPlayer->Stop();
		m_Mode = EditorMode::M_Edit;
	}

	void EditorApplication::TogglePause()
	{
		m_pEditorInstance->m_pPlayer->TogglePauze();
	}

	void EditorApplication::TickFrame()
	{
		if (m_Mode != EditorMode::M_Play) return;
		m_pEditorInstance->m_pPlayer->TickFrame();
	}

	bool EditorApplication::IsPaused()
	{
		return m_pEditorInstance->m_pPlayer->m_IsPaused;
	}

	void EditorApplication::RenderEditor()
	{
		m_pMainEditor->PaintEditor();
		//ImGui::ShowDemoWindow();
		//ImPlot::ShowDemoWindow();
	}

	void EditorApplication::InitializePlatform(Game& game)
	{
		m_pPlatform->Initialize(game);
		m_pMainEditor = new MainEditor();
		m_pMainEditor->Initialize();

		InitializeExtensions();

		m_pPlayer = new EditorPlayer();

		Debug::LogInfo("Initialized editor platform");
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
		Debug::LogInfo(stream.str());
		m_pEditorInstance->m_pMainEditor->OnFileDragAndDrop(path);
	}
}

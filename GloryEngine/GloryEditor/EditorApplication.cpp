#include "EditorApplication.h"
#include <imgui.h>
#include <Console.h>
#include <implot.h>

namespace Glory::Editor
{
	EditorApplication* EditorApplication::m_pEditorInstance = nullptr;
	const Glory::Version EditorApplication::Version(VERSION_DATA, 3);
	Mode EditorApplication::m_Mode = Mode::M_Edit;

	GLORY_EDITOR_API EditorApplication::EditorApplication(const EditorCreateInfo& createInfo)
		: m_pMainEditor(nullptr), m_pPlatform(nullptr), m_pTempWindowImpl(createInfo.pWindowImpl),
		m_pTempRenderImpl(createInfo.pRenderImpl), m_pShaderProcessor(nullptr), m_pPlayer(nullptr),
		m_pModulesLoopHandler(new EditorModuleLoopHandler())
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

	GLORY_EDITOR_API EditorApplication::~EditorApplication()
	{
		delete m_pMainEditor;
		m_pMainEditor = nullptr;

		delete m_pPlatform;
		m_pPlatform = nullptr;

		delete m_pModulesLoopHandler;
		m_pModulesLoopHandler = nullptr;
	}

	GLORY_EDITOR_API void EditorApplication::Initialize(Game& game)
	{
		game.OverrideAssetPathFunc(EditorApplication::AssetPathOverrider);

		m_pPlatform = new EditorPlatform(m_pTempWindowImpl, m_pTempRenderImpl);
		m_pTempWindowImpl->m_pEditorPlatform = m_pPlatform;
		m_pTempRenderImpl->m_pEditorPlatform = m_pPlatform;
		m_pTempWindowImpl = nullptr;
		m_pTempRenderImpl = nullptr;

		InitializePlatform(game);
	}

	void EditorApplication::InitializeExtensions()
	{
		for (size_t i = 0; i < m_pExtensions.size(); i++)
		{
			m_pExtensions[i]->RegisterEditors();
			m_pExtensions[i]->SetCurrentContext();
		}
	}

	GLORY_EDITOR_API void EditorApplication::Destroy()
	{
		m_pMainEditor->Destroy();
		m_pPlatform->Destroy();
		m_pShaderProcessor->Stop();
		delete m_pShaderProcessor;
		m_pShaderProcessor = nullptr;

		delete m_pPlayer;
		m_pPlayer = nullptr;
	}

	GLORY_EDITOR_API void EditorApplication::Run(Game& game)
	{
		//game.GetEngine()->Initialize();
		game.GetEngine()->StartThreads();
		m_pPlatform->SetState(Idle);
		m_pShaderProcessor->Start();

		while (true)
		{
			// Start a frame
			game.GetEngine()->GameThreadFrameStart();
			// Update console
			Console::Update();

			// Poll window events
			if (m_pPlatform->PollEvents()) break;

			// Update editor
			m_pMainEditor->Update();

			// Update engine (this also does the render loop)
			game.GetEngine()->ModulesLoop(m_pModulesLoopHandler);

			// End the current frame
			game.GetEngine()->GameThreadFrameEnd();

			// We need to wait for the frame to start its rendering
			m_pPlatform->Wait(Begin);
			// Render the editor (imgui calls)
			RenderEditor();
			// Now we notify the editor platform it can perform rendering
			m_pPlatform->SetState(Idle);
			// Wait for the end of rendering
			m_pPlatform->Wait(End);
			// Sync
			m_pPlatform->SetState(Idle);
		}
	}

	GLORY_EDITOR_API void EditorApplication::SetWindowImpl(EditorWindowImpl* pWindowImpl)
	{
		m_pTempWindowImpl = pWindowImpl;
	}

	GLORY_EDITOR_API void EditorApplication::SetRendererImpl(EditorRenderImpl* pRendererImpl)
	{
		m_pTempRenderImpl = pRendererImpl;
	}

	GLORY_EDITOR_API EditorPlatform* EditorApplication::GetEditorPlatform()
	{
		return m_pPlatform;
	}

	GLORY_EDITOR_API MainEditor* EditorApplication::GetMainEditor()
	{
		return m_pMainEditor;
	}

	GLORY_EDITOR_API EditorApplication* EditorApplication::GetInstance()
	{
		return m_pEditorInstance;
	}

	const Mode& EditorApplication::CurrentMode()
	{
		return m_Mode;
	}

	void EditorApplication::StartPlay()
	{
		if (m_Mode != Mode::M_Edit) return;
		m_Mode = Mode::M_EnteringPlay;
		m_pEditorInstance->m_pPlayer->Start();
		m_Mode = Mode::M_Play;
	}

	void EditorApplication::StopPlay()
	{
		if (m_Mode != Mode::M_Play) return;
		m_Mode = Mode::M_ExitingPlay;
		m_pEditorInstance->m_pPlayer->Stop();
		m_Mode = Mode::M_Edit;
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
		m_pEditorInstance = this;

		InitializeExtensions();

		m_pShaderProcessor = new EditorShaderProcessor();
		m_pPlayer = new EditorPlayer();
	}

	std::string EditorApplication::AssetPathOverrider()
	{
		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		if (pProject == nullptr) return std::string("./Assets");
		std::filesystem::path path = pProject->RootPath();
		path.append("Assets");
		return path.string();
	}
}

#include "EditorPlatform.h"

namespace Glory::Editor
{
	EditorPlatform::EditorPlatform(EditorWindowImpl* pWindowImpl, EditorRenderImpl* pRenderImpl)
		: m_pWindowImpl(pWindowImpl), m_pRenderImpl(pRenderImpl)
	{
	}

	EditorPlatform::~EditorPlatform()
	{
		delete m_pWindowImpl;
		m_pWindowImpl = nullptr;
		delete m_pRenderImpl;
		m_pRenderImpl = nullptr;
	}

	void EditorPlatform::Initialize(Game& game)
	{
		m_pWindowImpl->Initialize();
		SetupDearImGuiContext();
		m_pRenderImpl->Initialize();
		m_pRenderImpl->SetupBackend();
		m_pRenderImpl->UploadImGUIFonts();

		game.GetEngine()->GetGraphicsThread()->BindRenderOnly<EditorPlatform>(this);
	}

	bool EditorPlatform::PollEvents()
	{
		return m_pWindowImpl->PollEvents();
	}

	void EditorPlatform::BeginRender()
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_RenderState = Begin;
		lock.unlock();
	}

	void EditorPlatform::EndRender()
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_RenderState = End;
		lock.unlock();
	}

	void EditorPlatform::Destroy()
	{
		m_pRenderImpl->Shutdown();
		m_pWindowImpl->Shutdown();
		m_pRenderImpl->Cleanup();
		ImGui::DestroyContext();
	}

	void EditorPlatform::WaitIdle()
	{
		while (true)
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			EditorRenderState renderState = m_RenderState;
			lock.unlock();
			if (renderState == Idle) return;
		}
	}

	EditorWindowImpl* EditorPlatform::GetWindowImpl()
	{
		return m_pWindowImpl;
	}

	EditorRenderImpl* EditorPlatform::GetRenderImpl()
	{
		return m_pRenderImpl;
	}

	void EditorPlatform::SetupDearImGuiContext()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
	}

	void EditorPlatform::NewFrame()
	{
		m_pRenderImpl->NewFrame();
		m_pWindowImpl->NewFrame();
		ImGui::NewFrame();
	}

	void EditorPlatform::Render(const RenderFrame&)
	{
		std::unique_lock<std::mutex> lock(m_Mutex);

		switch (m_RenderState)
		{
		case Glory::Editor::Idle:
			return;
		case Glory::Editor::Begin:
			HandleBeginRender();
			m_RenderState = Idle;
			break;
		case Glory::Editor::End:
			HandleEndRender();
			m_RenderState = Idle;
			break;
		default:
			break;
		}

		lock.unlock();
	}

	void EditorPlatform::HandleBeginRender()
	{
		m_pRenderImpl->BeforeRender();
		NewFrame();
	}

	void EditorPlatform::HandleEndRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		// Rendering
		ImGui::Render();

		ImDrawData* main_draw_data = ImGui::GetDrawData();
		const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);

		m_pRenderImpl->Clear(CLEARCOLOR);

		if (!main_is_minimized)
			m_pRenderImpl->FrameRender(main_draw_data);

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		// Present Main Platform Window
		if (!main_is_minimized)
			m_pRenderImpl->FramePresent();
	}
}
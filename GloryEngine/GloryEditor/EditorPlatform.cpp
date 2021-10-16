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

	void EditorPlatform::Initialize()
	{
		m_pWindowImpl->Initialize();
		m_pRenderImpl->Initialize();
		SetupDearImGuiContext();
		m_pRenderImpl->SetupBackend();
		m_pRenderImpl->UploadImGUIFonts();
	}

	bool EditorPlatform::BeginRender()
	{
		if (m_pWindowImpl->PollEvents()) return true;
		m_pRenderImpl->BeforeRender();
		NewFrame();
		return false;
	}

	void EditorPlatform::EndRender()
	{
		Render();
	}

	void EditorPlatform::Destroy()
	{
		m_pRenderImpl->Shutdown();
		m_pWindowImpl->Shutdown();
		ImGui::DestroyContext();
		m_pRenderImpl->Cleanup();
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
		m_pWindowImpl->NewFrame();
		m_pRenderImpl->NewFrame();
		ImGui::NewFrame();
	}

	void EditorPlatform::Render()
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
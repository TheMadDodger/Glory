#include "EditorPlatform.h"
#include "EditorAssets.h"
#include "EditorRenderImpl.h"
#include "EditorWindowImpl.h"

#include <implot.h>
#include <ImGuizmo.h>
#include <imgui_internal.h>
#include <Engine.h>
#include <GraphicsThread.h>

#include <IconsFontAwesome6.h>
#include <IconsFontAwesome6Brand.h>

namespace Glory::Editor
{
	ImFont* EditorPlatform::LargeFont;

	EditorPlatform::EditorPlatform(EditorWindowImpl* pWindowImpl, EditorRenderImpl* pRenderImpl)
		: m_pWindowImpl(pWindowImpl), m_pRenderImpl(pRenderImpl), m_RenderState(Initializing)
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
		m_pWindowImpl->SetContext(m_pImguiConext);
		m_pRenderImpl->SetContext(m_pImguiConext);

		LoadFonts();
		m_pRenderImpl->UploadImGUIFonts();

		game.GetEngine()->GetGraphicsThread()->BindInitializeOnly<EditorPlatform>(this);
		game.GetEngine()->GetGraphicsThread()->BindBeginAndEndRender<EditorPlatform>(this);
	}

	void EditorPlatform::ThreadedInitialize()
	{
		m_pRenderImpl->Initialize();
		m_pRenderImpl->SetupBackend();
	}

	bool EditorPlatform::PollEvents()
	{
		return m_pWindowImpl->PollEvents();
	}

	void EditorPlatform::Destroy()
	{
		m_pRenderImpl->Shutdown();
		m_pWindowImpl->Shutdown();
		m_pRenderImpl->Cleanup();
		ImPlot::DestroyContext();
		ImGui::DestroyContext();

		EditorAssets::Destroy();
	}

	void EditorPlatform::Wait(const EditorRenderState& waitState)
	{
		while (true)
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			EditorRenderState renderState = m_RenderState;
			lock.unlock();
			if (renderState == waitState) return;
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
		m_pImguiConext = ImGui::CreateContext();
		ImPlot::CreateContext();
		ImGuiIO& io = m_pImguiConext->IO;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
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
		ImGuizmo::BeginFrame();
		ImGuizmo::Enable(true);
	}

	void EditorPlatform::GraphicsThreadBeginRender()
	{
		EditorAssets::LoadAssets();

		Wait(Idle);

		m_pRenderImpl->BeforeRender();
		NewFrame();

		std::unique_lock<std::mutex> lock(m_Mutex);
		m_RenderState = Begin;
		lock.unlock();

		Wait(Idle);
	}

	void EditorPlatform::GraphicsThreadEndRender()
	{
		Wait(Idle);

		ImGuiIO& io = m_pImguiConext->IO;
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

		std::unique_lock<std::mutex> lock(m_Mutex);
		m_RenderState = End;
		lock.unlock();

		Wait(Idle);
	}

	void EditorPlatform::LoadFonts()
	{
		auto& io = ImGui::GetIO();
		ImFont* pFont = io.Fonts->AddFontFromFileTTF("./EditorAssets/Fonts/PT_Sans/PTSans-Regular.ttf", 18.0f);
		io.FontDefault = pFont;

		static const ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		static const ImWchar brandIconRanges[] = { ICON_MIN_FAB, ICON_MAX_FAB, 0 };

		ImFontConfig config;
		config.DstFont = pFont;
		config.MergeMode = true;
		config.PixelSnapH = true;
		config.GlyphMinAdvanceX = 14.0f;
		ImFont* pIconFont = io.Fonts->AddFontFromFileTTF("./EditorAssets/Fonts/FA/" FONT_ICON_FILE_NAME_FAS, 14.0f, &config, iconRanges);
		ImFont* pIconBrandsFont = io.Fonts->AddFontFromFileTTF("./EditorAssets/Fonts/FA/" FONT_ICON_FILE_NAME_FAB, 14.0f, &config, brandIconRanges);

		LargeFont = io.Fonts->AddFontFromFileTTF("./EditorAssets/Fonts/PT_Sans/PTSans-Regular.ttf", 24.0f);
	}

	void EditorPlatform::SetState(const EditorRenderState& state)
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_RenderState = state;
		lock.unlock();
	}
}
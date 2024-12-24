#pragma once
#include "GloryEditor.h"

#include <imgui.h>
#include <mutex>

namespace Glory
{
	class Engine;

namespace Editor
{
	class EditorRenderImpl;
	class EditorWindowImpl;

	class EditorPlatform
	{
	public:
		EditorPlatform(EditorWindowImpl* pWindowImpl, EditorRenderImpl* pRenderImpl);
		virtual ~EditorPlatform();

		void Initialize(Engine* pEngine);
		bool PollEvents();
		void Destroy();

		GLORY_EDITOR_API EditorWindowImpl* GetWindowImpl();
		GLORY_EDITOR_API EditorRenderImpl* GetRenderImpl();

		void BeginFrame();
		void EndFrame();

		static ImFont* LargeFont;

	private:
		void SetupDearImGuiContext();

		void NewFrame();

		void LoadFonts();

	private:
		friend class EditorApplication;
		EditorWindowImpl* m_pWindowImpl;
		EditorRenderImpl* m_pRenderImpl;

		ImGuiContext* m_pImguiConext;
		bool m_Windowless;

		const ImVec4 CLEARCOLOR = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	};
}
}

#pragma once
#include "EditorRenderImpl.h"
#include "EditorWindowImpl.h"
#include <imgui.h>
#include <Game.h>

namespace Glory::Editor
{
	enum EditorRenderState
	{
		Idle,
		Begin,
		End,
	};

	class EditorPlatform
	{
	public:
		EditorPlatform(EditorWindowImpl* pWindowImpl, EditorRenderImpl* pRenderImpl);
		virtual ~EditorPlatform();

		void Initialize(Game& game);
		void ThreadedInitialize();
		bool PollEvents();
		void BeginRender();
		void EndRender();
		void Destroy();

		void WaitIdle();

		EditorWindowImpl* GetWindowImpl();
		EditorRenderImpl* GetRenderImpl();

	private:
		void SetupDearImGuiContext();

		void NewFrame();
		void Render(const RenderFrame&);

		void HandleBeginRender();
		void HandleEndRender();
		void LoadFonts();

	private:
		friend class GraphicsThread;
		EditorWindowImpl* m_pWindowImpl;
		EditorRenderImpl* m_pRenderImpl;

		std::mutex m_Mutex;
		EditorRenderState m_RenderState;

		const ImVec4 CLEARCOLOR = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	};
}

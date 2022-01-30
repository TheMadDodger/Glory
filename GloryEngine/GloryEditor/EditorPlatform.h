#pragma once
#include "EditorRenderImpl.h"
#include "EditorWindowImpl.h"
#include <imgui.h>
#include <Game.h>

namespace Glory::Editor
{
	enum EditorRenderState
	{
		Initializing,
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
		void Destroy();

		void Wait(const EditorRenderState& waitState);

		EditorWindowImpl* GetWindowImpl();
		EditorRenderImpl* GetRenderImpl();

	private:
		void SetupDearImGuiContext();

		void NewFrame();

		void GraphicsThreadBeginRender();
		void GraphicsThreadEndRender();
		void LoadFonts();

		void SetState(const EditorRenderState& state);

	private:
		friend class GraphicsThread;
		friend class EditorApplication;
		EditorWindowImpl* m_pWindowImpl;
		EditorRenderImpl* m_pRenderImpl;

		std::mutex m_Mutex;
		EditorRenderState m_RenderState;

		const ImVec4 CLEARCOLOR = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	};
}

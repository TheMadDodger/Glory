#pragma once
#include "EditorRenderImpl.h"
#include "EditorWindowImpl.h"

namespace Glory::Editor
{
	class EditorPlatform
	{
	public:
		EditorPlatform(EditorWindowImpl* pWindowImpl, EditorRenderImpl* pRenderImpl);
		virtual ~EditorPlatform();

		void Initialize();
		bool BeginRender();
		void EndRender();
		void Destroy();

		EditorWindowImpl* GetWindowImpl();
		EditorRenderImpl* GetRenderImpl();

	private:
		void SetupDearImGuiContext();

		void NewFrame();
		void Render();

	private:
		EditorWindowImpl* m_pWindowImpl;
		EditorRenderImpl* m_pRenderImpl;

		const ImVec4 CLEARCOLOR = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	};
}

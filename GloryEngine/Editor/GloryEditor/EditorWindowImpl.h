#pragma once
#include <Window.h>
#include <imgui.h>
#include "GloryEditor.h"

namespace Glory::Editor
{
	class EditorPlatform;

	class EditorWindowImpl
	{
	public:
		GLORY_EDITOR_API EditorWindowImpl();
		virtual GLORY_EDITOR_API ~EditorWindowImpl();

		virtual void SetContext(ImGuiContext* pImguiConext) = 0;
		GLORY_EDITOR_API void Initialize();

		virtual void Shutdown() = 0;

		virtual void SetupForOpenGL() = 0;
		virtual void SetupForVulkan() = 0;
		virtual void SetupForD3D() = 0;
		virtual void SetupForMetal() = 0;

		GLORY_EDITOR_API Window* GetMainWindow();

	protected:
		virtual bool PollEvents() = 0;

		virtual void NewFrame() = 0;

	protected:
		Window* m_pMainWindow;
		EditorPlatform* m_pEditorPlatform;

	private:
		friend class EditorPlatform;
		friend class EditorApplication;
	};
}

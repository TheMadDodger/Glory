#pragma once
#include <imgui.h>
#include <Texture.h>
#include "GloryEditor.h"

namespace Glory::Editor
{
	class EditorPlatform;

	class EditorRenderImpl
	{
	public:
		GLORY_EDITOR_API EditorRenderImpl();
		GLORY_EDITOR_API virtual ~EditorRenderImpl();

		virtual void* GetTextureID(Texture* pTexture) = 0;

	protected:
		virtual void SetContext(ImGuiContext* pImguiConext) = 0;
		virtual void Setup() = 0;
		virtual void SetupBackend() = 0;
		virtual void UploadImGUIFonts() = 0;

		virtual void Shutdown() = 0;
		virtual void Cleanup() = 0;

		virtual void BeforeRender() = 0;
		virtual void NewFrame() = 0;
		virtual void Clear(const ImVec4& clearColor) = 0;
		virtual void FrameRender(ImDrawData* pDrawData) = 0;
		virtual void FramePresent() = 0;

	protected:
		EditorPlatform* m_pEditorPlatform;

	private:
		void Initialize();
		void Render();
		void Destroy();

	private:
		void SetupDearImGuiContext();

	private:
		friend class EditorPlatform;
		friend class EditorApplication;
	};
}

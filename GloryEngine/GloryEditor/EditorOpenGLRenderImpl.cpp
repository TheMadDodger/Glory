#include "EditorOpenGLRenderImpl.h"
#include <Game.h>
#include <GL/glew.h>
#include <GLTexture.h>
#include <ImFileDialog.h>

namespace Glory::Editor
{
	EditorOpenGLRenderImpl::EditorOpenGLRenderImpl() {}

	EditorOpenGLRenderImpl::~EditorOpenGLRenderImpl() {}

	void EditorOpenGLRenderImpl::Setup()
	{
		m_pEditorPlatform->GetWindowImpl()->SetupForOpenGL();
	}

	void EditorOpenGLRenderImpl::SetupBackend()
	{
		const char* glsl_version = "#version 130";
		ImGui_ImplOpenGL3_Init(glsl_version);

		ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void* {
			GLuint tex;

			glGenTextures(1, &tex);
			glBindTexture(GL_TEXTURE_2D, tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, (fmt == 0) ? GL_BGRA : GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);

			return (void*)tex;
		};
		ifd::FileDialog::Instance().DeleteTexture = [](void* tex) {
			GLuint texID = (GLuint)tex;
			glDeleteTextures(1, &texID);
		};
	}

	void EditorOpenGLRenderImpl::UploadImGUIFonts()
	{
		
	}

	void EditorOpenGLRenderImpl::Shutdown()
	{
		
	}

	void EditorOpenGLRenderImpl::Cleanup()
	{
		ImGui_ImplOpenGL3_Shutdown();
	}

	void EditorOpenGLRenderImpl::BeforeRender()
	{
		
	}

	void EditorOpenGLRenderImpl::NewFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
	}

	void EditorOpenGLRenderImpl::Clear(const ImVec4& clearColor)
	{
		int display_w, display_h;
		m_pEditorPlatform->GetWindowImpl()->GetMainWindow()->GetDrawableSize(&display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		//glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
		//glClear(GL_COLOR_BUFFER_BIT);
	}

	void EditorOpenGLRenderImpl::FrameRender(ImDrawData* pDrawData)
	{
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void EditorOpenGLRenderImpl::FramePresent()
	{
		m_pEditorPlatform->GetWindowImpl()->GetMainWindow()->GLSwapWindow();
	}

	void* EditorOpenGLRenderImpl::GetTextureID(Texture* pTexture)
	{
		if (pTexture == nullptr) return 0;
		GLTexture* pGLTexture = (GLTexture*)pTexture;
		return (void*)pGLTexture->GetID();
	}
}

#include "OGLRenderTexture.h"
#include "OpenGLGraphicsModule.h"
#include "Debug.h"
#include <GL/glew.h>
#include <Game.h>

namespace Glory
{
	GLuint OGLRenderTexture::GetDepthBuffer()
	{
		return m_GLDepthBufferID;
	}

	OGLRenderTexture::OGLRenderTexture(int width, int height, bool hasDepthBuffer) : RenderTexture(width, height, hasDepthBuffer)
	{
	}

	OGLRenderTexture::~OGLRenderTexture()
	{
		if (m_GLFrameBufferID != NULL) glDeleteFramebuffers(1, &m_GLFrameBufferID);
		if (m_GLDepthBufferID != NULL) glDeleteRenderbuffers(1, &m_GLDepthBufferID);
		m_GLFrameBufferID = NULL;
		m_GLDepthBufferID = NULL;
	}

	void OGLRenderTexture::Initialize()
	{
		// Create framebuffer
		glGenFramebuffers(1, &m_GLFrameBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindFramebuffer(GL_FRAMEBUFFER, m_GLFrameBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		
		GPUResourceManager* pResourceManager = Game::GetGame().GetEngine()->GetGraphicsModule()->GetResourceManager();
		m_pTexture = pResourceManager->CreateTexture(m_Width, m_Height, PixelFormat::PF_R8G8B8A8Srgb, ImageType::IT_2D, 0, 0, ImageAspect::IA_Color, SamplerSettings());
		
		GLTexture* pGLTexture = (GLTexture*)m_pTexture;
		glBindTexture(GL_TEXTURE_2D, pGLTexture->GetID());
		OpenGLGraphicsModule::LogGLError(glGetError());

		pGLTexture->InitializeEmpty();

		// Make the depth buffer
		if (m_HasDepthBuffer)
		{
			// The depth buffer
			//glGenRenderbuffers(1, &m_GLDepthBufferID);
			//OpenGLGraphicsModule::LogGLError(glGetError());
			//glBindRenderbuffer(GL_RENDERBUFFER, m_GLDepthBufferID);
			//OpenGLGraphicsModule::LogGLError(glGetError());
			//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
			//OpenGLGraphicsModule::LogGLError(glGetError());
			//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_GLDepthBufferID);
			//OpenGLGraphicsModule::LogGLError(glGetError());

			glGenTextures(1, &m_GLDepthBufferID);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glBindTexture(GL_TEXTURE_2D, m_GLDepthBufferID);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}
		
		// Initialize the framebuffer
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, pGLTexture->GetID(), 0);
		OpenGLGraphicsModule::LogGLError(glGetError());
		
		if (m_HasDepthBuffer)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GLDepthBufferID, 0);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers);
		OpenGLGraphicsModule::LogGLError(glGetError());
		
		// Check if something went wrong
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			Debug::LogError("There was an error when trying to create a frame buffer!");
			return;
		}

		glBindTexture(GL_TEXTURE_2D, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLRenderTexture::OnResize()
	{
		if (m_GLFrameBufferID != NULL) glDeleteFramebuffers(1, &m_GLFrameBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		if (m_GLDepthBufferID != NULL) glDeleteRenderbuffers(1, &m_GLDepthBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		m_GLFrameBufferID = NULL;
		m_GLDepthBufferID = NULL;

		GPUResourceManager* pResourceManager = Game::GetGame().GetEngine()->GetGraphicsModule()->GetResourceManager();
		pResourceManager->Free(m_pTexture);

		Initialize();
	}

	void OGLRenderTexture::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_GLFrameBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glViewport(0, 0, m_Width, m_Height);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}
	
	void OGLRenderTexture::UnBind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}
}

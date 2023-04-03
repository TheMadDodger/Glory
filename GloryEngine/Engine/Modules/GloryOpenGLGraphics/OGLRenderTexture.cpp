#include "OGLRenderTexture.h"
#include "OpenGLGraphicsModule.h"
#include "Debug.h"
#include <GL/glew.h>
#include <Game.h>
#include <Engine.h>
#include "GloryOGL.h"

namespace Glory
{
	OGLRenderTexture::OGLRenderTexture(const RenderTextureCreateInfo& createInfo) : RenderTexture(createInfo), m_GLFrameBufferID(0)
	{
	}

	OGLRenderTexture::~OGLRenderTexture()
	{
		if (m_GLFrameBufferID != NULL) glDeleteFramebuffers(1, &m_GLFrameBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		m_GLFrameBufferID = NULL;
	}

	uint32_t OGLRenderTexture::ReadPixel(const glm::ivec2& coord)
	{
		Bind();

		uint32_t index = (uint32_t)m_NameToTextureIndex["object"];
		uint32_t value;
		glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
		glReadPixels(coord.x, coord.y, 1, 1, GL_RED_INTEGER, GL_INT, &value);

		UnBind();
		return value;
	}

	void OGLRenderTexture::Initialize()
	{
		// Create framebuffer
		glGenFramebuffers(1, &m_GLFrameBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindFramebuffer(GL_FRAMEBUFFER, m_GLFrameBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		GPUResourceManager* pResourceManager = Game::GetGame().GetEngine()->GetGraphicsModule()->GetResourceManager();
		CreateTextures(pResourceManager);
		
		//GLTexture* pGLTexture = (GLTexture*)m_pTexture;
		//glBindTexture(GL_TEXTURE_2D, pGLTexture->GetID());
		//OpenGLGraphicsModule::LogGLError(glGetError());

		// Make the depth buffer
		//if (m_HasDepthBuffer)
		//{
		//	// The depth buffer
		//	//glGenRenderbuffers(1, &m_GLDepthBufferID);
		//	//OpenGLGraphicsModule::LogGLError(glGetError());
		//	//glBindRenderbuffer(GL_RENDERBUFFER, m_GLDepthBufferID);
		//	//OpenGLGraphicsModule::LogGLError(glGetError());
		//	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
		//	//OpenGLGraphicsModule::LogGLError(glGetError());
		//	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_GLDepthBufferID);
		//	//OpenGLGraphicsModule::LogGLError(glGetError());
		//
		//	glGenTextures(1, &m_GLDepthBufferID);
		//	OpenGLGraphicsModule::LogGLError(glGetError());
		//	glBindTexture(GL_TEXTURE_2D, m_GLDepthBufferID);
		//	OpenGLGraphicsModule::LogGLError(glGetError());
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//	OpenGLGraphicsModule::LogGLError(glGetError());
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//	OpenGLGraphicsModule::LogGLError(glGetError());
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//	OpenGLGraphicsModule::LogGLError(glGetError());
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//	OpenGLGraphicsModule::LogGLError(glGetError());
		//	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
		//	OpenGLGraphicsModule::LogGLError(glGetError());
		//}
		
		// Initialize the framebuffer
		size_t attachmentCount = m_CreateInfo.Attachments.size();
		std::vector<GLenum> drawBuffers = std::vector<GLenum>(attachmentCount);
		for (uint32_t i = 0; i < attachmentCount; i++)
		{
			drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
			GLTexture* pTexture = (GLTexture*)m_pTextures[i];
			glFramebufferTexture(GL_FRAMEBUFFER, drawBuffers[i], pTexture->GetID(), 0);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}
		
		if (m_CreateInfo.HasDepth)
		{
			GLTexture* pDepthTexture = (GLTexture*)GetTextureAttachment("Depth");
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, pDepthTexture->GetID(), 0);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		glDrawBuffers(attachmentCount, &drawBuffers[0]);
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
		//if (m_GLDepthBufferID != NULL) glDeleteRenderbuffers(1, &m_GLDepthBufferID);
		//OpenGLGraphicsModule::LogGLError(glGetError());
		m_GLFrameBufferID = NULL;
		//m_GLDepthBufferID = NULL;

		GPUResourceManager* pResourceManager = Game::GetGame().GetEngine()->GetGraphicsModule()->GetResourceManager();
		size_t attachmentCount = m_CreateInfo.Attachments.size();
		for (size_t i = 0; i < attachmentCount; i++)
		{
			pResourceManager->Free(m_pTextures[i]);
		}
		if (m_CreateInfo.HasDepth) pResourceManager->Free(m_pTextures[attachmentCount]);
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

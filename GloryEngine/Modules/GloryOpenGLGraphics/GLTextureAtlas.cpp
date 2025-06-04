#include "GLTextureAtlas.h"
#include "GLTexture.h"
#include "OpenGLGraphicsModule.h"

#include <Engine.h>
#include <Debug.h>
#include "GLConverter.h"

namespace Glory
{
	GLTextureAtlas::GLTextureAtlas(TextureCreateInfo&& createInfo, Engine* pEngine):
		TextureAtlas(pEngine, createInfo.m_Width, createInfo.m_Height), m_pTexture(nullptr), m_GLFrameBufferID(0)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResources = pGraphics->GetResourceManager();
		m_pTexture = static_cast<GLTexture*>(pResources->CreateTexture(std::move(createInfo)));
	}

	GLTextureAtlas::~GLTextureAtlas()
	{
		m_pTexture = nullptr;
	}

	void GLTextureAtlas::Initialize()
	{
		// Create framebuffer
		glGenFramebuffers(1, &m_GLFrameBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindFramebuffer(GL_FRAMEBUFFER, m_GLFrameBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		// Initialize the framebuffer
		const GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
		glFramebufferTexture(GL_FRAMEBUFFER, drawBuffer, m_pTexture->GetID(), 0);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glDrawBuffers(1, &drawBuffer);
		OpenGLGraphicsModule::LogGLError(glGetError());

		// Check if something went wrong
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			m_pEngine->GetDebug().LogError("There was an error when trying to create a frame buffer!");
			return;
		}

		glBindTexture(GL_TEXTURE_2D, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	Texture* GLTextureAtlas::GetTexture()
	{
		return m_pTexture;
	}

	bool GLTextureAtlas::AssignChunk(Texture* pTexture, const ReservedChunk& chunk)
	{
		OpenGLGraphicsModule* pGraphics = static_cast<OpenGLGraphicsModule*>(m_pEngine->GetMainModule<GraphicsModule>());
		Material* pMaterial = pGraphics->UsePassthroughMaterial();

		glBindFramebuffer(GL_FRAMEBUFFER, m_GLFrameBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glViewport(chunk.XOffset, chunk.YOffset, chunk.Width, chunk.Height);
		OpenGLGraphicsModule::LogGLError(glGetError());

		pMaterial->SetTexture("Color", pTexture);
		pGraphics->DrawScreenQuad();

		pGraphics->UseMaterial(nullptr);
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		return true;
	}
}

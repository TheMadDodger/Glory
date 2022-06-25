#include "GLTexture.h"
#include "OpenGLGraphicsModule.h"
#include "GLConverter.h"
#include <Game.h>
#include "GloryOGL.h"

namespace Glory
{
	GLTexture::GLTexture(uint32_t width, uint32_t height, const PixelFormat& format, const PixelFormat& internalFormat, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings)
		: Texture(width, height, format, internalFormat, imageType, usageFlags, sharingMode, imageAspectFlags, samplerSettings),
		m_TextureID(0), m_GLImageType(0)
	{
	}

	GLTexture::~GLTexture()
	{
		glDeleteTextures(1, &m_TextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		m_TextureID = NULL;
	}

	GLuint GLTexture::GetGLImageType() const
	{
		return m_GLImageType;
	}

	GLuint GLTexture::GetID() const
	{
		return m_TextureID;
	}

	void GLTexture::Create(ImageData* pImageData)
	{
		m_GLImageType = GLConverter::GetGLImageType(m_ImageType);

		GLuint internalFormat = GLConverter::TO_GLFORMAT.at(m_InternalFormat);
		GLuint format = GLConverter::TO_GLFORMAT.at(m_PixelFormat);
		//glTexImage2D(m_GLImageType, 0, format, (GLsizei)pImageData->GetWidth(), (GLsizei)pImageData->GetHeight(), 0, format, GL_FLOAT, pImageData->GetPixels());
		//OpenGLGraphicsModule::LogGLError(glGetError());
		//
		//glTexParameteri(m_GLImageType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//OpenGLGraphicsModule::LogGLError(glGetError());
		//glTexParameteri(m_GLImageType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//OpenGLGraphicsModule::LogGLError(glGetError());

		glGenTextures(1, &m_TextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindTexture(m_GLImageType, m_TextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glTexImage2D(m_GLImageType, 0, internalFormat, (GLsizei)pImageData->GetWidth(), (GLsizei)pImageData->GetHeight(), 0, format, GL_UNSIGNED_BYTE, pImageData->GetPixels());
		OpenGLGraphicsModule::LogGLError(glGetError());

		glTexParameteri(m_GLImageType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(m_GLImageType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void GLTexture::Create()
	{
		m_GLImageType = GLConverter::GetGLImageType(m_ImageType);

		GLuint format = GLConverter::TO_GLFORMAT.at(m_PixelFormat);
		GLuint internalFormat = GLConverter::TO_GLFORMAT.at(m_InternalFormat);

		glGenTextures(1, &m_TextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindTexture(m_GLImageType, m_TextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		// Initialize texture
		glTexImage2D(m_GLImageType, 0, internalFormat, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glTexParameteri(m_GLImageType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(m_GLImageType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(m_GLImageType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(m_GLImageType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void GLTexture::CopyFromBuffer(Buffer* pBuffer, int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth)
	{
	}
}

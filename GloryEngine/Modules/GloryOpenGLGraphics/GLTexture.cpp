#include "GLTexture.h"
#include "OpenGLGraphicsModule.h"
#include "GLConverter.h"
#include "GloryOGL.h"
#include <Engine.h>

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

	void GLTexture::Create(TextureData* pTextureData)
	{
		Resource* pParent = pTextureData->ParentResource();
		ImageData* pParentImage = pParent ? dynamic_cast<ImageData*>(pParent) : nullptr;
		ImageData* pImageData = pParentImage ? pParentImage : pTextureData->Image().Get(&m_pOwner->GetEngine()->GetResources());
		if (!pImageData) return;

		m_GLImageType = GLConverter::GetGLImageType(m_ImageType);

		GLuint internalFormat = GLConverter::TO_GLFORMAT.at(m_InternalFormat);
		GLuint format = GLConverter::TO_GLFORMAT.at(m_PixelFormat);

		if (!m_TextureID)
		{
			glGenTextures(1, &m_TextureID);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		glBindTexture(m_GLImageType, m_TextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glTexImage2D(m_GLImageType, 0, internalFormat, (GLsizei)pImageData->GetWidth(), (GLsizei)pImageData->GetHeight(), 0, format, GL_UNSIGNED_BYTE, pImageData->GetPixels());
		OpenGLGraphicsModule::LogGLError(glGetError());

		SamplerSettings& sampler = pTextureData->GetSamplerSettings();
		glTexParameteri(m_GLImageType, GL_TEXTURE_MIN_FILTER, GLConverter::TO_GLFILTER.at(sampler.MinFilter));
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(m_GLImageType, GL_TEXTURE_MAG_FILTER, GLConverter::TO_GLFILTER.at(sampler.MagFilter));
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeU));
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeV));
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeW));
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindTexture(m_GLImageType, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	GLORY_API void GLTexture::Create()
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindTexture(m_GLImageType, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	GLORY_API void GLTexture::CopyFromBuffer(Buffer* pBuffer, int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth)
	{
	}
}

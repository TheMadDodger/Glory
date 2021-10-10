#include "GLTexture.h"
#include "OpenGLGraphicsModule.h"
#include "GLConverter.h"

namespace Glory
{
	GLTexture::GLTexture(uint32_t width, uint32_t height, const PixelFormat& format, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings)
		: Texture(width, height, format, imageType, usageFlags, sharingMode, imageAspectFlags, samplerSettings)
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

		//glGenTextures(1, &m_TextureID);
		//OpenGLGraphicsModule::LogGLError(glGetError());
		//glBindTexture(m_GLImageType, m_TextureID);
		//OpenGLGraphicsModule::LogGLError(glGetError());

		GLuint format = GLConverter::GetGLFormat(m_PixelFormat);
		//glTexImage2D(m_GLImageType, 0, format, (GLsizei)pImageData->GetWidth(), (GLsizei)pImageData->GetHeight(), 0, format, GL_FLOAT, pImageData->GetPixels());
		//OpenGLGraphicsModule::LogGLError(glGetError());
		//
		//glTexParameteri(m_GLImageType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//OpenGLGraphicsModule::LogGLError(glGetError());
		//glTexParameteri(m_GLImageType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//OpenGLGraphicsModule::LogGLError(glGetError());

		//GLenum pixelFormat{ GL_RGB };
		//switch (pImageData->GetBytesPerPixel())
		//{
		//case 3:
		//	if (m_pImage->format->Rmask == 0x000000ff)
		//	{
		//		pixelFormat = GL_RGB;
		//	}
		//	else
		//	{
		//		pixelFormat = GL_BGR;
		//	}
		//	break;
		//case 4:
		//	if (m_pImage->format->Rmask == 0x000000ff)
		//	{
		//		pixelFormat = GL_RGBA;
		//	}
		//	else
		//	{
		//		pixelFormat = GL_BGRA;
		//	}
		//	break;
		//default:
		//	//std::cerr << "Texture::CreateFromSurface, unknow pixel format, BytesPerPixel: " << m_pImage->format->BytesPerPixel << "\nUse 32 bit or 24 bit images.\n";
		//	return;
		//}

		//m_Dimensions = Math::Vector2((float)m_pImage->w, (float)m_pImage->h);

		glGenTextures(1, &m_TextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindTexture(m_GLImageType, m_TextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glTexImage2D(m_GLImageType, 0, GL_RGBA, (GLsizei)pImageData->GetWidth(), (GLsizei)pImageData->GetHeight(), 0, format, GL_UNSIGNED_BYTE, pImageData->GetPixels());
		OpenGLGraphicsModule::LogGLError(glGetError());

		glTexParameteri(m_GLImageType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(m_GLImageType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void GLTexture::Create()
	{
	}

	void GLTexture::CopyFromBuffer(Buffer* pBuffer, int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth)
	{
	}
}

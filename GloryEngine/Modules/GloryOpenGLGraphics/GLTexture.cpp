#include "GLTexture.h"
#include "OpenGLGraphicsModule.h"
#include "GLConverter.h"
#include "GloryOGL.h"

#include <CubemapData.h>
#include <Engine.h>

namespace Glory
{
	GLTexture::GLTexture(TextureCreateInfo&& textureInfo)
		: Texture(std::move(textureInfo)),
		m_TextureID(0), m_GLImageType(0), m_BindlessHandle(0)
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

	GLuint64 GLTexture::GetBindlessID() const
	{
		return m_BindlessHandle;
	}

	void GLTexture::Create(TextureData* pTextureData)
	{
		ImageData* pImageData = pTextureData->GetImageData(&m_pOwner->GetEngine()->GetAssetManager());
		if (!pImageData) return;

		m_GLImageType = GLConverter::GetGLImageType(m_TextureInfo.m_ImageType);

		const GLuint format = GLConverter::TO_GLFORMAT.at(m_TextureInfo.m_PixelFormat);
		const GLuint internalFormat = m_TextureInfo.m_PixelFormat == PixelFormat::PF_Stencil ? GL_STENCIL_INDEX8 :
			GLConverter::TO_GLFORMAT.at(m_TextureInfo.m_InternalFormat);
		const GLenum dataType = GLConverter::TO_GLDATATYPE.at(m_TextureInfo.m_Type);

		if (pImageData->GetBytesPerPixel() == 1)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (m_TextureID)
		{
			glDeleteTextures(1, &m_TextureID);
			OpenGLGraphicsModule::LogGLError(glGetError());
			m_TextureID = NULL;
		}

		if (!m_TextureID)
		{
			glGenTextures(1, &m_TextureID);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		glBindTexture(m_GLImageType, m_TextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glTexImage2D(m_GLImageType, 0, internalFormat, (GLsizei)pImageData->GetWidth(), (GLsizei)pImageData->GetHeight(), 0, format, dataType, pImageData->GetPixels());
		OpenGLGraphicsModule::LogGLError(glGetError());

		SamplerSettings& sampler = pTextureData->GetSamplerSettings();
		if (sampler.MipmapMode != Filter::F_None)
		{
			glGenerateMipmap(m_GLImageType);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		const GLint minFilter = GLConverter::GetMinFilter(sampler.MipmapMode, sampler.MinFilter);

		glTexParameteri(m_GLImageType, GL_TEXTURE_MIN_FILTER, minFilter);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(m_GLImageType, GL_TEXTURE_MAG_FILTER, GLConverter::TO_GLFILTER.at(sampler.MagFilter));
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(m_GLImageType, GL_TEXTURE_WRAP_S, GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeU));
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(m_GLImageType, GL_TEXTURE_WRAP_T, GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeV));
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(m_GLImageType, GL_TEXTURE_WRAP_R, GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeW));
		OpenGLGraphicsModule::LogGLError(glGetError());

		float aniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameterf(m_GLImageType, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());

		m_BindlessHandle = glGetTextureHandleARB(m_TextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glMakeTextureHandleResidentARB(m_BindlessHandle);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindTexture(m_GLImageType, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void GLTexture::Create(CubemapData* pCubemapData)
	{
		ImageData* pImageData = pCubemapData->GetImageData(&m_pOwner->GetEngine()->GetAssetManager(), 0);
		if (!pImageData) return;

		m_GLImageType = GLConverter::GetGLImageType(m_TextureInfo.m_ImageType);

		const GLuint format = GLConverter::TO_GLFORMAT.at(m_TextureInfo.m_PixelFormat);
		const GLuint internalFormat = m_TextureInfo.m_PixelFormat == PixelFormat::PF_Stencil ? GL_STENCIL_INDEX8 :
			GLConverter::TO_GLFORMAT.at(m_TextureInfo.m_InternalFormat);
		const GLenum dataType = GLConverter::TO_GLDATATYPE.at(m_TextureInfo.m_Type);

		if (pImageData->GetBytesPerPixel() == 1)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (m_TextureID)
		{
			glDeleteTextures(1, &m_TextureID);
			OpenGLGraphicsModule::LogGLError(glGetError());
			m_TextureID = NULL;
		}

		if (!m_TextureID)
		{
			glGenTextures(1, &m_TextureID);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		glBindTexture(m_GLImageType, m_TextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		SamplerSettings& sampler = pCubemapData->GetSamplerSettings();
		for (unsigned int i = 0; i < 6; ++i)
		{
			ImageData* pImageData = pCubemapData->GetImageData(&m_pOwner->GetEngine()->GetAssetManager(), i);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat,
				(GLsizei)pImageData->GetWidth(), (GLsizei)pImageData->GetHeight(), 0, format, dataType, pImageData->GetPixels());
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		const GLint minFilter = GLConverter::GetMinFilter(sampler.MipmapMode, sampler.MinFilter);

		glTexParameteri(m_GLImageType, GL_TEXTURE_MIN_FILTER, minFilter);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(m_GLImageType, GL_TEXTURE_MAG_FILTER, GLConverter::TO_GLFILTER.at(sampler.MagFilter));
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(m_GLImageType, GL_TEXTURE_WRAP_S, GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeU));
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(m_GLImageType, GL_TEXTURE_WRAP_T, GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeV));
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(m_GLImageType, GL_TEXTURE_WRAP_R, GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeW));
		OpenGLGraphicsModule::LogGLError(glGetError());

		float aniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameterf(m_GLImageType, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());

		if (sampler.MipmapMode != Filter::F_None)
		{
			glGenerateMipmap(m_GLImageType);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		m_BindlessHandle = glGetTextureHandleARB(m_TextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glMakeTextureHandleResidentARB(m_BindlessHandle);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindTexture(m_GLImageType, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void GLTexture::Create(const void* pixels)
	{
		m_GLImageType = GLConverter::GetGLImageType(m_TextureInfo.m_ImageType);

		const GLuint format = GLConverter::TO_GLFORMAT.at(m_TextureInfo.m_PixelFormat);
		const GLuint internalFormat = m_TextureInfo.m_PixelFormat == PixelFormat::PF_Stencil ? GL_STENCIL_INDEX8 :
			GLConverter::TO_GLFORMAT.at(m_TextureInfo.m_InternalFormat);
		const GLenum dataType = GLConverter::TO_GLDATATYPE.at(m_TextureInfo.m_Type);

		glGenTextures(1, &m_TextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindTexture(m_GLImageType, m_TextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		// Initialize texture
		glTexImage2D(m_GLImageType, 0, internalFormat, m_TextureInfo.m_Width, m_TextureInfo.m_Height, 0, format, dataType, pixels);
		OpenGLGraphicsModule::LogGLError(glGetError());

		SamplerSettings& sampler = m_TextureInfo.m_SamplerSettings;
		if (sampler.MipmapMode != Filter::F_None)
		{
			glGenerateMipmap(m_GLImageType);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		const GLint minFilter = GLConverter::GetMinFilter(sampler.MipmapMode, sampler.MinFilter);

		glTexParameteri(m_GLImageType, GL_TEXTURE_MIN_FILTER, minFilter);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(m_GLImageType, GL_TEXTURE_MAG_FILTER, GLConverter::TO_GLFILTER.at(sampler.MagFilter));
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeU));
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeV));
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeW));
		OpenGLGraphicsModule::LogGLError(glGetError());

		float aniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());

		m_BindlessHandle = glGetTextureHandleARB(m_TextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glMakeTextureHandleResidentARB(m_BindlessHandle);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindTexture(m_GLImageType, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void GLTexture::CopyFromBuffer(Buffer* pBuffer, int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth)
	{
	}

	uint64_t GLTexture::BindlessHandle() const
	{
		return m_BindlessHandle;
	}
}

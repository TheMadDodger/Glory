#pragma once
#include <Texture.h>
#include <GL/glew.h>
#include <Glory.h>

namespace Glory
{
	class GLTexture : public Texture
	{
	public:
		GLTexture(uint32_t width, uint32_t height, const PixelFormat& format, const PixelFormat& internalFormat, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings = SamplerSettings());
		virtual ~GLTexture();

		GLORY_API GLuint GetGLImageType() const;
		GLORY_API GLuint GetID() const;

	private:
		GLORY_API virtual void Create(ImageData* pImageData) override;
		GLORY_API virtual void Create() override;
		GLORY_API virtual void CopyFromBuffer(Buffer* pBuffer, int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth) override;

	private:
		GLuint m_TextureID;
		GLuint m_GLImageType;
	};
}

#pragma once
#include <Texture.h>
#include <GL/glew.h>

namespace Glory
{
	class GLTexture : public Texture
	{
	public:
		GLTexture(uint32_t width, uint32_t height, const PixelFormat& format, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings = SamplerSettings());
		virtual ~GLTexture();

		GLuint GetGLImageType() const;
		GLuint GetID() const;

		void InitializeEmpty();

	private:
		virtual void Create(ImageData* pImageData) override;
		virtual void Create() override;
		virtual void CopyFromBuffer(Buffer* pBuffer, int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth) override;

	private:
		GLuint m_TextureID;
		GLuint m_GLImageType;
	};
}

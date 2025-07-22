#pragma once
#include <Texture.h>
#include <GL/glew.h>
#include <Glory.h>

namespace Glory
{
	class CubemapData;

	class GLTexture : public Texture
	{
	public:
		GLTexture(TextureCreateInfo&& textureInfo);
		virtual ~GLTexture();

		GLORY_API GLuint GetGLImageType() const;
		GLORY_API GLuint GetID() const;
		GLORY_API GLuint64 GetBindlessID() const;

	private:
		GLORY_API virtual void Create(TextureData* pTextureData) override;
		GLORY_API virtual void Create(CubemapData* pCubemapData) override;
		GLORY_API virtual void Create(const void* pixels=nullptr) override;
		GLORY_API virtual void CopyFromBuffer(Buffer* pBuffer, int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth) override;
		GLORY_API virtual uint64_t BindlessHandle() const override;

	private:
		GLuint m_TextureID;
		GLuint m_GLImageType;
		GLuint64 m_BindlessHandle;
	};
}

#include "Texture.h"

namespace Glory
{
	Texture::Texture(TextureCreateInfo&& textureInfo)
		: m_TextureInfo(std::move(textureInfo))
	{
	}

	Texture::~Texture()
	{
	}

	void Texture::CopyFromBuffer(Buffer* pBuffer)
	{
		CopyFromBuffer(pBuffer, 0, 0, 0, m_TextureInfo.m_Width, m_TextureInfo.m_Height, 1);
	}
}

#include "Texture.h"

namespace Glory
{
	Texture::Texture(uint32_t width, uint32_t height, const PixelFormat& format, const ImageType& imageType, uint32_t usageFlags,
		uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings)
		: m_Width(width), m_Height(height), m_ImageType(imageType), m_UsageFlags(usageFlags), m_SharingMode(sharingMode),
		m_ImageAspectFlags(imageAspectFlags), m_SamplerSettings(samplerSettings), m_PixelFormat(format)
	{
	}

	Texture::~Texture()
	{
	}

	void Texture::CopyFromBuffer(Buffer* pBuffer)
	{
		CopyFromBuffer(pBuffer, 0, 0, 0, m_Width, m_Height, 1);
	}
}

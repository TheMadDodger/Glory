#include "Texture.h"

namespace Glory
{
	Texture::Texture(ImageData* pImageData, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings)
		: m_pImageData(pImageData), m_ImageType(imageType), m_UsageFlags(usageFlags), m_SharingMode(sharingMode), m_ImageAspectFlags(imageAspectFlags), m_SamplerSettings(samplerSettings)
	{
	}

	Texture::~Texture()
	{
	}

	void Texture::CopyFromBuffer(Buffer* pBuffer)
	{
		CopyFromBuffer(pBuffer, 0, 0, 0, m_pImageData->GetWidth(), m_pImageData->GetHeight(), 1);
	}
}

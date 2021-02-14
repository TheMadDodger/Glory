#pragma once
#include "ImageData.h"
#include "GraphicsEnums.h"

namespace Glory
{
	class Buffer;

	class Texture
	{
	public:
		Texture(ImageData* pImageData, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings = SamplerSettings());
		virtual ~Texture();

		virtual void Create() = 0;
		void CopyFromBuffer(Buffer* pBuffer);
		virtual void CopyFromBuffer(Buffer* pBuffer, int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth) = 0;

	protected:
		ImageData* m_pImageData;
		ImageType m_ImageType;
		uint32_t m_UsageFlags;
		uint32_t m_SharingMode;
		ImageAspect m_ImageAspectFlags;
		SamplerSettings m_SamplerSettings;
	};
}

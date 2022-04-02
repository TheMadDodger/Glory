#pragma once
#include "ImageData.h"
#include "GraphicsEnums.h"
#include "GPUResource.h"
#include <any>
#include <glm/glm.hpp>

namespace Glory
{
	class Buffer;

	class Texture : public GPUResource
	{
	public:
		Texture(uint32_t width, uint32_t height, const PixelFormat& format, const PixelFormat& internalFormat, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings = SamplerSettings());
		virtual ~Texture();

		virtual void Create(ImageData* pImageData) = 0;
		virtual void Create() = 0;
		void CopyFromBuffer(Buffer* pBuffer);
		virtual void CopyFromBuffer(Buffer* pBuffer, int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth) = 0;

	protected:
		ImageType m_ImageType;
		uint32_t m_UsageFlags;
		uint32_t m_SharingMode;
		uint32_t m_Width;
		uint32_t m_Height;
		ImageAspect m_ImageAspectFlags;
		SamplerSettings m_SamplerSettings;
		PixelFormat m_InternalFormat;
		PixelFormat m_PixelFormat;
	};
}

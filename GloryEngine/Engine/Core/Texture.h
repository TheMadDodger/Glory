#pragma once
#include "TextureData.h"
#include "GraphicsEnums.h"
#include "GPUResource.h"
#include <any>
#include <glm/glm.hpp>

namespace Glory
{
	class Buffer;

	struct TextureCreateInfo
	{
		uint32_t m_Width;
		uint32_t m_Height;
		PixelFormat m_PixelFormat;
		PixelFormat m_InternalFormat;
		ImageType m_ImageType;
		DataType m_Type;
		uint32_t m_UsageFlags;
		uint32_t m_SharingMode;
		ImageAspect m_ImageAspectFlags;
		SamplerSettings m_SamplerSettings = SamplerSettings();
	};

	class Texture : public GPUResource
	{
	public:
		Texture(TextureCreateInfo&& textureInfo);
		virtual ~Texture();

		virtual void Create(TextureData* pTextureData) = 0;
		virtual void Create() = 0;
		void CopyFromBuffer(Buffer* pBuffer);
		virtual void CopyFromBuffer(Buffer* pBuffer, int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth) = 0;

	protected:
		TextureCreateInfo m_TextureInfo;
	};
}

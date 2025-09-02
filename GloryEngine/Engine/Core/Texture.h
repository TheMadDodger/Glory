#pragma once
#include "TextureData.h"
#include "GraphicsEnums.h"
#include "GPUResource.h"
#include <any>
#include <glm/glm.hpp>

namespace Glory
{
	class Buffer;
	class CubemapData;

	class Texture : public GPUResource
	{
	public:
		Texture(TextureCreateInfo&& textureInfo);
		virtual ~Texture();

		virtual void Create(TextureData* pTextureData) = 0;
		virtual void Create(CubemapData* pCubemapData) = 0;
		virtual void Create(const void* pixels=nullptr) = 0;
		void CopyFromBuffer(Buffer* pBuffer);
		virtual void CopyFromBuffer(Buffer* pBuffer, int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth) = 0;

		const TextureCreateInfo& Info() const;

	protected:
		TextureCreateInfo m_TextureInfo;
	};
}

#pragma once
#include "TextureData.h"

#include <engine_visibility.h>

namespace Glory
{
	class ImageData;

	class InternalTexture : public TextureData
	{
	public:
		GLORY_ENGINE_API InternalTexture();
		GLORY_ENGINE_API InternalTexture(ImageData* pImage, bool owning=true);
		GLORY_ENGINE_API InternalTexture(InternalTexture&&) noexcept = default;
		GLORY_ENGINE_API InternalTexture& operator=(InternalTexture&&) noexcept = default;
		GLORY_ENGINE_API virtual ~InternalTexture();

		GLORY_ENGINE_API virtual ImageData* GetImageData(Resources* pAssetManager);

		GLORY_ENGINE_API void Serialize(Utils::BinaryStream& container) const override;
		GLORY_ENGINE_API void Deserialize(Utils::BinaryStream& container) override;

		GLORY_ENGINE_API void SetImage(ImageData* pImage, bool owning=true);

	private:
		ImageData* m_pImage;
		bool m_Owning;
	};
}

#pragma once
#include "TextureData.h"

namespace Glory
{
	class ImageData;

	class InternalTexture : public TextureData
	{
	public:
		InternalTexture();
		InternalTexture(ImageData* pImage);
		virtual ~InternalTexture();

		virtual ImageData* GetImageData(AssetManager* pAssetManager);

		GLORY_API void Serialize(BinaryStream& container) const override;
		GLORY_API void Deserialize(BinaryStream& container) override;

	private:
		ImageData* m_pImage;
	};
}

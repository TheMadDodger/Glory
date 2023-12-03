#pragma once
#include "ImageData.h"
#include "AssetReference.h"

namespace Glory
{
	class TextureData : public Resource
	{
    public:
        TextureData();
        TextureData(ImageData* pImageData);
        virtual ~TextureData();

        AssetReference<ImageData>& Image();
        SamplerSettings& GetSamplerSettings();

        void Serialize(BinaryStream& container) const override;
        void Deserialize(BinaryStream& container) override;

    private:
        friend class TextureDataLoaderModule;
        friend class ImageLoaderModule;
        AssetReference<ImageData> m_Image;
        SamplerSettings m_SamplerSettings;
	};
}

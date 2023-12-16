#pragma once
#include "ImageData.h"
#include "AssetReference.h"

namespace Glory
{
	class TextureData : public Resource
	{
    public:
        TextureData();
        TextureData(UUID uuid, const std::string& name);
        TextureData(ImageData* pImageData);
        virtual ~TextureData();

        AssetReference<ImageData>& Image();
        SamplerSettings& GetSamplerSettings();

        void Serialize(BinaryStream& container) const override;
        void Deserialize(BinaryStream& container) override;

    private:
        AssetReference<ImageData> m_Image;
        SamplerSettings m_SamplerSettings;
	};
}

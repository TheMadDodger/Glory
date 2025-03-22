#pragma once
#include "TextureData.h"

namespace Glory
{
	class CubemapData : public TextureData
	{
    public:
        CubemapData();
        CubemapData(UUID right, UUID left, UUID down, UUID up, UUID front, UUID back);
        CubemapData(ImageData* pRight, ImageData* pLeft, ImageData* pDown, ImageData* pUp, ImageData* pFront, ImageData* pBack);
        virtual ~CubemapData();

        virtual ImageData* GetImageData(AssetManager* pAssetManager, size_t face);
        AssetReference<ImageData>& Image(size_t face);
        SamplerSettings& GetSamplerSettings();

        void Serialize(BinaryStream& container) const override;
        void Deserialize(BinaryStream& container) override;

        void References(Engine* pEngine, std::vector<UUID>& references) const override;

    private:
        friend class TextureDataLoaderModule;
        friend class ImageLoaderModule;
        AssetReference<ImageData> m_Faces[6];
        SamplerSettings m_SamplerSettings;
	};
}

#pragma once
#include "ImageData.h"
#include "AssetReference.h"

namespace Glory
{
    class AssetManager;
    class Engine;

	class TextureData : public Resource
	{
    public:
        TextureData();
        TextureData(ImageData* pImageData);
        virtual ~TextureData();

        virtual ImageData* GetImageData(AssetManager* pAssetManager);
        AssetReference<ImageData>& Image();
        SamplerSettings& GetSamplerSettings();

        void Serialize(BinaryStream& container) const override;
        void Deserialize(BinaryStream& container) override;

        void References(Engine* pEngine, std::vector<UUID>& references) const override;

    private:
        friend class TextureDataLoaderModule;
        friend class ImageLoaderModule;
        AssetReference<ImageData> m_Image;
        SamplerSettings m_SamplerSettings;
	};
}

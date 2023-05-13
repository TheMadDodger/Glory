#include "TextureDataLoaderModule.h"
#include "NodeRef.h"

namespace Glory
{
    TextureDataLoaderModule::TextureDataLoaderModule() : ResourceLoaderModule(".gtex")
    {
    }

    TextureDataLoaderModule::~TextureDataLoaderModule()
    {
    }

    const std::type_info& TextureDataLoaderModule::GetModuleType()
    {
        return typeid(TextureDataLoaderModule);
    }

    TextureData* TextureDataLoaderModule::LoadResource(const std::string& path, const ImportSettings&)
    {
        YAMLFileRef file{ path };
        file.Load();

        const UUID imageUUID = file["Image"].As<uint64_t>();

        TextureData* pTexture = new TextureData();
        pTexture->m_Image.SetUUID(imageUUID);

        return pTexture;
    }

    TextureData* TextureDataLoaderModule::LoadResource(const void* buffer, size_t length, const ImportSettings&)
    {
        return nullptr;
    }

    void TextureDataLoaderModule::SaveResource(const std::string& path, TextureData* pResource)
    {
        YAMLFileRef file{ path };
        file["Image"].Set((uint64_t)pResource->Image().AssetUUID());
        file.Save();
    }
}

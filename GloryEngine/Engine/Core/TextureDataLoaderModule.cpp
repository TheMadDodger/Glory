#include "TextureDataLoaderModule.h"
#include "NodeRef.h"

using namespace Glory::Utils;

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
        Utils::YAMLFileRef file{ path };
        file.Load();

        const UUID imageUUID = file["Image"].As<uint64_t>();

        TextureData* pTexture = new TextureData();
        pTexture->m_Image.SetUUID(imageUUID);

        Utils::NodeValueRef sampler = file["Sampler"];
        pTexture->m_SamplerSettings.MagFilter = sampler["MinFilter"].AsEnum<Filter>();
        pTexture->m_SamplerSettings.MinFilter = sampler["MagFilter"].AsEnum<Filter>();
        pTexture->m_SamplerSettings.AddressModeU = sampler["AddressModeU"].AsEnum<SamplerAddressMode>();
        pTexture->m_SamplerSettings.AddressModeV = sampler["AddressModeV"].AsEnum<SamplerAddressMode>();
        pTexture->m_SamplerSettings.AddressModeW = sampler["AddressModeW"].AsEnum<SamplerAddressMode>();
        pTexture->m_SamplerSettings.AnisotropyEnable = sampler["AnisotropyEnable"].As<bool>();
        pTexture->m_SamplerSettings.MaxAnisotropy = sampler["MaxAnisotropy"].As<float>();
        pTexture->m_SamplerSettings.UnnormalizedCoordinates = sampler["UnnormalizedCoordinates"].As<bool>();
        pTexture->m_SamplerSettings.CompareEnable = sampler["CompareEnable"].As<bool>();
        pTexture->m_SamplerSettings.CompareOp = sampler["CompareOp"].AsEnum<CompareOp>();
        pTexture->m_SamplerSettings.MipmapMode = sampler["MipmapMode"].AsEnum<Filter>();
        pTexture->m_SamplerSettings.MipLODBias = sampler["MipLODBias"].As<float>();
        pTexture->m_SamplerSettings.MinLOD = sampler["MinLOD"].As<float>();
        pTexture->m_SamplerSettings.MaxLOD = sampler["MaxLOD"].As<float>();

        return pTexture;
    }

    TextureData* TextureDataLoaderModule::LoadResource(const void* buffer, size_t length, const ImportSettings&)
    {
        return nullptr;
    }

    void TextureDataLoaderModule::SaveResource(const std::string& path, TextureData* pResource)
    {
        Utils::YAMLFileRef file{ path };
        file["Image"].Set((uint64_t)pResource->Image().AssetUUID());
        Utils::NodeValueRef sampler = file["Sampler"];

        sampler["MinFilter"].SetEnum<Filter>(pResource->m_SamplerSettings.MagFilter);
        sampler["MagFilter"].SetEnum<Filter>(pResource->m_SamplerSettings.MinFilter);
        sampler["AddressModeU"].SetEnum<SamplerAddressMode>(pResource->m_SamplerSettings.AddressModeU);
        sampler["AddressModeV"].SetEnum<SamplerAddressMode>(pResource->m_SamplerSettings.AddressModeV);
        sampler["AddressModeW"].SetEnum<SamplerAddressMode>(pResource->m_SamplerSettings.AddressModeW);
        sampler["AnisotropyEnable"].Set(pResource->m_SamplerSettings.AnisotropyEnable);
        sampler["MaxAnisotropy"].Set(pResource->m_SamplerSettings.MaxAnisotropy);
        sampler["UnnormalizedCoordinates"].Set(pResource->m_SamplerSettings.UnnormalizedCoordinates);
        sampler["CompareEnable"].Set(pResource->m_SamplerSettings.CompareEnable);
        sampler["CompareOp"].SetEnum<CompareOp>(pResource->m_SamplerSettings.CompareOp);
        sampler["MipmapMode"].SetEnum<Filter>(pResource->m_SamplerSettings.MipmapMode);
        sampler["MipLODBias"].Set(pResource->m_SamplerSettings.MipLODBias);
        sampler["MinLOD"].Set(pResource->m_SamplerSettings.MinLOD);
        sampler["MaxLOD"].Set(pResource->m_SamplerSettings.MaxLOD);

        file.Save();
    }
}

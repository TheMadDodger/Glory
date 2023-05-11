#include "TextureDataLoaderModule.h"

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
        return nullptr;
    }

    TextureData* TextureDataLoaderModule::LoadResource(const void* buffer, size_t length, const ImportSettings&)
    {
        return nullptr;
    }
}

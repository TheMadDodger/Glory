#include "ImageLoaderModule.h"
#include <string>

namespace Glory
{
	ImageLoaderModule::ImageLoaderModule()
	{
	}

	ImageLoaderModule::~ImageLoaderModule()
	{
	}

	ImageData* ImageLoaderModule::LoadResource(const std::string& path, const ImageImportSettings&)
	{
		ImageData* pTexture = LoadTexture(path);
		pTexture->BuildTexture();
		return pTexture;
	}

	ImageData* ImageLoaderModule::LoadResource(const void* buffer, size_t length, const ImageImportSettings& importSettings)
	{
		ImageData* pTexture = LoadTexture(buffer, length, importSettings);
		if (!pTexture) return nullptr;
		pTexture->BuildTexture();
		return pTexture;
	}

	const std::type_info& ImageLoaderModule::GetModuleType()
	{
		return typeid(ImageLoaderModule);
	}
}

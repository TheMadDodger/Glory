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

	ImageData* ImageLoaderModule::LoadResource(const std::string& path)
	{
		ImageData* pTexture = LoadTexture(path);
		pTexture->BuildTexture();
		return pTexture;
	}

	const std::type_info& ImageLoaderModule::GetModuleType()
	{
		return typeid(ImageLoaderModule);
	}
}

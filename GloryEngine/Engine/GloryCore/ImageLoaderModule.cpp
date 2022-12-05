#include "ImageLoaderModule.h"
#include "EngineProfiler.h"
#include <string>

namespace Glory
{
	ImageLoaderModule::ImageLoaderModule() : ResourceLoaderModule(".png,.jpg,.bmp")
	{
	}

	ImageLoaderModule::~ImageLoaderModule()
	{
	}

	ImageData* ImageLoaderModule::LoadResource(const std::string& path, const ImageImportSettings&)
	{
		Profiler::BeginSample("ImageLoaderModule::LoadResource(path)");
		ImageData* pTexture = LoadTexture(path);
		pTexture->BuildTexture();
		Profiler::EndSample();
		return pTexture;
	}

	ImageData* ImageLoaderModule::LoadResource(const void* buffer, size_t length, const ImageImportSettings& importSettings)
	{
		Profiler::BeginSample("ImageLoaderModule::LoadResource(buffer)");
		ImageData* pTexture = LoadTexture(buffer, length, importSettings);
		if (!pTexture)
		{
			Profiler::EndSample();
			return nullptr;
		}
		pTexture->BuildTexture();
		Profiler::EndSample();
		return pTexture;
	}

	const std::type_info& ImageLoaderModule::GetModuleType()
	{
		return typeid(ImageLoaderModule);
	}

	ImageImportSettings ImageLoaderModule::ReadImportSettings_Internal(YAML::Node& node)
	{
		return ImageImportSettings();
	}

	void ImageLoaderModule::WriteImportSettings_Internal(const ImageImportSettings& importSettings, YAML::Emitter& out)
	{
	}

	ImageImportSettings::ImageImportSettings() {}

	ImageImportSettings::ImageImportSettings(const std::string& extension)
		: ImportSettings(extension) {}
}

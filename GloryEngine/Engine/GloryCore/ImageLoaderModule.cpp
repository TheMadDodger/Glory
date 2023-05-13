#include "ImageLoaderModule.h"
#include "EngineProfiler.h"
#include "TextureData.h"

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
		ImageData* pImage = LoadTexture(path);
		pImage->BuildTexture();

		/* Default TextureData */
		TextureData* pDefualtTexture = new TextureData();
		pDefualtTexture->m_Image = pImage;
		pImage->AddSubresource(pDefualtTexture, "Default");

		Profiler::EndSample();
		return pImage;
	}

	ImageData* ImageLoaderModule::LoadResource(const void* buffer, size_t length, const ImageImportSettings& importSettings)
	{
		Profiler::BeginSample("ImageLoaderModule::LoadResource(buffer)");
		ImageData* pImage = LoadTexture(buffer, length, importSettings);
		if (!pImage)
		{
			Profiler::EndSample();
			return nullptr;
		}
		pImage->BuildTexture();

		/* Default TextureData */
		TextureData* pDefualtTexture = new TextureData();
		pDefualtTexture->m_Image = pImage;
		pImage->AddSubresource(pDefualtTexture, "Default");

		Profiler::EndSample();
		return pImage;
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

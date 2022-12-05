#pragma once
#include "ResourceLoaderModule.h"
#include "ImageData.h"

namespace Glory
{
	struct ImageImportSettings : ImportSettings
	{
		ImageImportSettings();
		ImageImportSettings(const std::string& extension);
	};

	class ImageLoaderModule : public ResourceLoaderModule<ImageData, ImageImportSettings>
	{
	public:
		ImageLoaderModule();
		virtual ~ImageLoaderModule();

		virtual const std::type_info& GetModuleType() override;

	protected:
		virtual ImageData* LoadTexture(const std::string& path) = 0;
		virtual ImageData* LoadTexture(const void* buffer, size_t length, const ImageImportSettings& importSettings) = 0;

		virtual ImageImportSettings ReadImportSettings_Internal(YAML::Node& node) override;
		virtual void WriteImportSettings_Internal(const ImageImportSettings& importSettings, YAML::Emitter& out) override;

	private:
		virtual ImageData* LoadResource(const std::string& path, const ImageImportSettings&) override;
		virtual ImageData* LoadResource(const void* buffer, size_t length, const ImageImportSettings& importSettings) override;

	protected:
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
	};
}

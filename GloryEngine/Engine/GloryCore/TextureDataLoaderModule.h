#pragma once
#include "ResourceLoaderModule.h"
#include "TextureData.h"

namespace Glory
{
	class TextureDataLoaderModule : public ResourceLoaderModule<TextureData, ImportSettings>
	{
	public:
		TextureDataLoaderModule();
		virtual ~TextureDataLoaderModule();

		virtual const std::type_info& GetModuleType() override;

	protected:
		virtual ImageData* LoadTexture(const std::string& path) = 0;
		virtual ImageData* LoadTexture(const void* buffer, size_t length, const ImportSettings& importSettings) = 0;

		virtual ImportSettings ReadImportSettings_Internal(YAML::Node& node) override { return ImportSettings(".gtex"); }
		virtual void WriteImportSettings_Internal(const ImportSettings& importSettings, YAML::Emitter& out) override {}

	private:
		virtual TextureData* LoadResource(const std::string& path, const ImportSettings&) override;
		virtual TextureData* LoadResource(const void* buffer, size_t length, const ImportSettings& importSettings) override;
	};
}

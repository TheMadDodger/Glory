#pragma once
#include "ResourceLoaderModule.h"
#include "ModelData.h"

namespace Glory
{
	struct ModelImportSettings : ImportSettings
	{
		ModelImportSettings();
		ModelImportSettings(const std::string& extension);
	};

	class ModelLoaderModule : public ResourceLoaderModule<ModelData, ModelImportSettings>
	{
	public:
		ModelLoaderModule();
		virtual ~ModelLoaderModule();

		virtual const std::type_info& GetModuleType() override;

	protected:
		virtual ModelData* LoadModel(const std::string& path, const ModelImportSettings& importSettings) = 0;
		virtual ModelData* LoadModel(const void* buffer, size_t length, const ModelImportSettings& importSettings) = 0;

		virtual ModelImportSettings ReadImportSettings_Internal(YAML::Node& node) override;
		virtual void WriteImportSettings_Internal(const ModelImportSettings& importSettings, YAML::Emitter& out) override;

	private:
		virtual ModelData* LoadResource(const std::string& path, const ModelImportSettings& importSettings) override;
		virtual ModelData* LoadResource(const void* buffer, size_t length, const ModelImportSettings& importSettings) override;

	protected:
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
	};
}

#pragma once
#include "ResourceLoaderModule.h"
#include "ModelData.h"

namespace Glory
{
	struct ModelImportSettings : ImportSettings
	{

	};

	class ModelLoaderModule : public ResourceLoaderModule<ModelData, ModelImportSettings>
	{
	public:
		ModelLoaderModule();
		virtual ~ModelLoaderModule();

		virtual const std::type_info& GetModuleType() override;

	protected:
		virtual ModelData* LoadModel(const std::string& path, const ModelImportSettings& importSettings) = 0;

	private:
		virtual ModelData* LoadResource(const std::string& path, const ModelImportSettings& importSettings) override;

	protected:
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
	};
}

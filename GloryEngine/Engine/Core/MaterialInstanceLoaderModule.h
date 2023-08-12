#pragma once
#include "ResourceLoaderModule.h"
#include "MaterialInstanceData.h"
#include "MaterialInstanceData.h"

namespace Glory
{
	struct MaterialInstanceImportSettings : public ImportSettings
	{
		MaterialInstanceImportSettings();
		MaterialInstanceImportSettings(const std::string& name);
	};

	class MaterialInstanceLoaderModule : public ResourceLoaderModule<MaterialInstanceData, MaterialInstanceImportSettings>
	{
	public:
		MaterialInstanceLoaderModule();
		virtual ~MaterialInstanceLoaderModule();

		virtual const std::type_info& GetModuleType() override;

	protected:
		virtual MaterialInstanceImportSettings ReadImportSettings_Internal(YAML::Node& node) override;
		virtual void WriteImportSettings_Internal(const MaterialInstanceImportSettings& importSettings, YAML::Emitter& out) override;

	private:
		virtual MaterialInstanceData* LoadResource(const std::string& path, const MaterialInstanceImportSettings& importSettings) override;
		virtual MaterialInstanceData* LoadResource(const void* buffer, size_t length, const MaterialInstanceImportSettings& importSettings) override;
		virtual void SaveResource(const std::string& path, MaterialInstanceData* pResource) override;

	private:
		MaterialInstanceData* LoadMaterialInstanceData(YAML::Node& rootNode, const MaterialInstanceImportSettings& importSettings);

		void SaveMaterialInstanceData(MaterialInstanceData* pMaterialData, YAML::Emitter& out);
		void ReadPropertyOverrides(YAML::Node& rootNode, MaterialInstanceData* pMaterialData);
		//void WritePropertyOverrides(YAML::Emitter& out, MaterialInstanceData* pMaterialData);

	protected:
		virtual void Initialize() override;
		virtual void Cleanup() override;
	};
}

#pragma once
#include <ImporterTemplate.h>
#include <MaterialInstanceData.h>
#include <yaml-cpp/yaml.h>

namespace Glory::Editor
{
	class MaterialInstanceImporter : public YAMLImporterTemplate<MaterialInstanceData>
	{
	public:
		MaterialInstanceImporter();
		virtual ~MaterialInstanceImporter();

		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual MaterialInstanceData* LoadResource(const std::filesystem::path& path) const override;
		bool SaveResource(const std::filesystem::path& path, MaterialInstanceData* pResource) const override;

		MaterialInstanceData* LoadMaterialInstanceData(YAML::Node& rootNode) const;

		void SaveMaterialInstanceData(MaterialInstanceData* pMaterialData, YAML::Emitter& out) const;
		void ReadPropertyOverrides(YAML::Node& rootNode, MaterialInstanceData* pMaterialData) const;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
	};
}

#pragma once
#include <ImporterTemplate.h>
#include <MaterialData.h>
#include <yaml-cpp/yaml.h>

namespace Glory::Editor
{
	class MaterialImporter : public YAMLImporterTemplate<MaterialData>
	{
	public:
		MaterialImporter();
		virtual ~MaterialImporter();

		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual ImportedResource LoadResource(const std::filesystem::path& path, void*) const override;
		bool SaveResource(const std::filesystem::path& path, MaterialData* pResource) const override;

		void SaveMaterialData(MaterialData* pMaterialData, Utils::NodeValueRef data) const;
		void WritePropertyData(Utils::NodeValueRef data, MaterialData* pMaterialData) const;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
	};
}

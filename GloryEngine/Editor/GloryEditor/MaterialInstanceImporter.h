#pragma once
#include <ImporterTemplate.h>
#include <MaterialInstanceData.h>
#include <yaml-cpp/yaml.h>

namespace Glory::Utils
{
	struct YAMLFileRef;
}

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

		MaterialInstanceData* LoadMaterialInstanceData(Utils::YAMLFileRef& file) const;

		void SaveMaterialInstanceData(MaterialInstanceData* pMaterialData, YAML::Emitter& out) const;
		void ReadPropertyOverrides(Utils::YAMLFileRef& file, MaterialInstanceData* pMaterialData) const;

	private:
		friend class EditorMaterialManager;
		virtual void Initialize() override;
		virtual void Cleanup() override;
	};
}

#pragma once
#include <ImporterTemplate.h>

#include <SoundMaterialData.h>

namespace Glory::Editor
{
    class SoundMaterialImporter : public YAMLImporterTemplate<SoundMaterialData>
    {
	public:
		SoundMaterialImporter() {};
		virtual ~SoundMaterialImporter() {};

		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual ImportedResource LoadResource(const std::filesystem::path& path, void*) const override;
		virtual bool SaveResource(const std::filesystem::path& path, SoundMaterialData* pScript) const override;

	private:
		virtual void Initialize() override {};
		virtual void Cleanup() override {};
    };
}

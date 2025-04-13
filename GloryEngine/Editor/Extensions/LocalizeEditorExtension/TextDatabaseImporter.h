#pragma once
#include <ImporterTemplate.h>
#include <TextDatabase.h>
#include <Glory.h>

namespace Glory::Editor
{
    class TextDatabaseImporter : public YAMLImporterTemplate<TextDatabase>
    {
	public:
		TextDatabaseImporter() {};
		virtual ~TextDatabaseImporter() {};

		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual ImportedResource LoadResource(const std::filesystem::path& path, void*) const override;
		virtual bool SaveResource(const std::filesystem::path& path, TextDatabase* pFSM) const override;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override {};
    };
}

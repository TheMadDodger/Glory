#pragma once
#include <ImporterTemplate.h>
#include <UIDocumentData.h>

namespace Glory::Editor
{
    class UIDocumentImporter : public ImporterTemplate<UIDocumentData>
    {
	public:
		UIDocumentImporter() {};
		virtual ~UIDocumentImporter() {};

		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual ImportedResource LoadResource(const std::filesystem::path& path, void*) const override;
		virtual bool SaveResource(const std::filesystem::path& path, UIDocumentData* pDocument) const override;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override {};
    };
}

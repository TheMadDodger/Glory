#pragma once
#include <ImporterTemplate.h>
#include <FontData.h>
#include <NodeRef.h>

namespace Glory::Editor
{
	class FontImporter : public ImporterTemplate<FontData>
	{
	public:
		FontImporter() {};
		virtual ~FontImporter() {};

		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual ImportedResource LoadResource(const std::filesystem::path& path, void*) const override;

	private:
		virtual void Initialize() override {};
		virtual void Cleanup() override {};
	};
}

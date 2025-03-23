#pragma once
#include <ImporterTemplate.h>
#include <ImageData.h>

namespace Glory::Editor
{
	class STBHDRImageImporter : public ImporterTemplate<ImageData>
	{
	public:
		STBHDRImageImporter();
		virtual ~STBHDRImageImporter();

		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual ImportedResource LoadResource(const std::filesystem::path& path, void* userData) const override;
		virtual ImportedResource LoadResource(void* data, size_t dataSize, void* userData) const override;
		virtual bool SaveResource(const std::filesystem::path& path, ImageData* pResource) const override;

		virtual void Initialize() override {}
		virtual void Cleanup() override {}
	};
}

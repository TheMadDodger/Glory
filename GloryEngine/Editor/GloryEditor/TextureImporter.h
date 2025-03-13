#pragma once
#include "ImporterTemplate.h"

#include <TextureData.h>

namespace Glory::Editor
{
	class TextureImporter : public YAMLImporterTemplate<TextureData>
	{
	public:
		TextureImporter();
		virtual ~TextureImporter();

		std::string_view Name() const override;

		static void LoadIntoTexture(Utils::YAMLFileRef file, TextureData* pTexture);

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual ImportedResource LoadResource(const std::filesystem::path& path, void*) const override;
		bool SaveResource(const std::filesystem::path& path, TextureData* pResource) const override;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
	};
}

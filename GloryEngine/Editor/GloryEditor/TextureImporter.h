#pragma once
#include "ImporterTemplate.h"

#include <TextureData.h>

namespace Glory::Editor
{
	class TextureImporter : public ImporterTemplate<TextureData>
	{
	public:
		TextureImporter();
		virtual ~TextureImporter();

		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual TextureData* LoadResource(const std::filesystem::path& path) const override;
		bool SaveResource(const std::filesystem::path& path, TextureData* pResource) const override;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
	};
}

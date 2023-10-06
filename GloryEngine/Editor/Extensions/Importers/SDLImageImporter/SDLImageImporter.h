#pragma once
#include <ImporterTemplate.h>
#include <ImageData.h>

namespace Glory::Editor
{
	class SDLImageImporter : public ImporterTemplate<ImageData>
	{
	public:
		SDLImageImporter();
		virtual ~SDLImageImporter();

		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual ImageData* LoadResource(const std::filesystem::path& path) const override;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;

		int m_InitializedFlags;
	};
}

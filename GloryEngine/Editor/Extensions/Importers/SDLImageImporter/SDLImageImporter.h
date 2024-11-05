#pragma once
#include <ImporterTemplate.h>
#include <ImageData.h>

struct SDL_Surface;

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
		virtual ImportedResource LoadResource(const std::filesystem::path& path, void* userData) const override;
		virtual ImportedResource LoadResource(void* data, size_t dataSize, void* userData) const override;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;

		ImportedResource Process(const std::filesystem::path& path, SDL_Surface* pSDLImage) const;

		int m_InitializedFlags;
	};
}

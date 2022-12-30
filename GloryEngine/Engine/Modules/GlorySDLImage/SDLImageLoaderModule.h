#pragma once
#include <ImageLoaderModule.h>

namespace Glory
{
	class SDLImageLoaderModule : public ImageLoaderModule
	{
	public:
		SDLImageLoaderModule();
		virtual ~SDLImageLoaderModule();

		GLORY_MODULE_VERSION_H;

	private:
		virtual ImageData* LoadTexture(const std::string& path) override;
		virtual ImageData* LoadTexture(const void* buffer, size_t length, const ImageImportSettings& importSettings) override;
		virtual bool HasPriority() override;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;

		int m_InitializedFlags;
	};
}

#pragma once
#include <ImageLoaderModule.h>

namespace Glory
{
	class SDLImageLoaderModule : public ImageLoaderModule
	{
	public:
		SDLImageLoaderModule();
		virtual ~SDLImageLoaderModule();

	private:
		virtual ImageData* LoadTexture(const std::string& path) override;
		virtual bool HasPriority() override;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;

		int m_InitializedFlags;
	};
}

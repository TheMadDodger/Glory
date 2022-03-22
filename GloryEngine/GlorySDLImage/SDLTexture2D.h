#pragma once
#include <ImageData.h>
#include <SDL2/SDL_surface.h>

namespace Glory
{
	class SDLImageData : public ImageData
	{
	public:
		SDLImageData();
		virtual ~SDLImageData();

		virtual const void* GetPixels() override;

	private:
		virtual void BuildTexture() override;

	private:
		friend class SDLImageLoaderModule;
		SDL_Surface* m_pSurface;
	};
}

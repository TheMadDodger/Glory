#include "SDLImageLoaderModule.h"
#include <SDL_image.h>
#include <Debug.h>
#include "SDLTexture2D.h"

namespace Glory
{
	SDLImageLoaderModule::SDLImageLoaderModule() : m_InitializedFlags(0)
	{
	}

	SDLImageLoaderModule::~SDLImageLoaderModule()
	{
	}

	ImageData* SDLImageLoaderModule::LoadTexture(const std::string& path)
	{
		SDL_Surface* pSDLImage = IMG_Load(path.data());

		if (pSDLImage == NULL)
		{
			const char* error = SDL_GetError();
			Debug::LogWarning("SDL Could not load image " + path + " SDL Error: " + std::string(error));

			return nullptr;
		}

		SDLImageData* pData = new SDLImageData();
		//pData->m_Origin = Vector2(pSDLImage->w / 2.0f, pSDLImage->h / 2.0f);
		pData->m_pSurface = pSDLImage;

		return pData;
	}

	ImageData* SDLImageLoaderModule::LoadTexture(const void* buffer, size_t length, const ImageImportSettings& importSettings)
	{
		SDL_RWops* rw = SDL_RWFromConstMem(buffer, length);

		SDL_Surface* pSDLImage;
		if (importSettings.m_Extension.length() > 0)
			pSDLImage = IMG_LoadTyped_RW(rw, 1, importSettings.m_Extension.c_str());
		else
			pSDLImage = IMG_Load_RW(rw, 1);

		if (pSDLImage == NULL)
		{
			const char* error = SDL_GetError();
			Debug::LogWarning("SDL Could not load image from memory SDL Error: " + std::string(error));

			return nullptr;
		}

		SDLImageData* pData = new SDLImageData();
		//pData->m_Origin = Vector2(pSDLImage->w / 2.0f, pSDLImage->h / 2.0f);
		pData->m_pSurface = pSDLImage;

		return pData;
	}

	bool SDLImageLoaderModule::HasPriority()
	{
		return false;
	}

	void SDLImageLoaderModule::Initialize()
	{
		// Initialize for all available extensions
		m_InitializedFlags = IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP);
		if (m_InitializedFlags == 0)
		{
			const char* error = SDL_GetError();
			Debug::LogFatalError("Could not initialize SDL_image, SDL Error: " + std::string(error));
		}
	}

	void SDLImageLoaderModule::Cleanup()
	{
		IMG_Quit();
	}
}

#include "SDLImageImporter.h"

#include <EditorApplication.h>
#include <SDL2/SDL_image.h>
#include <sstream>
#include <Debug.h>
#include <TextureData.h>

namespace Glory::Editor
{
	constexpr size_t NumSupportedExtensions = 10;
	constexpr std::string_view SupportedExtensions[NumSupportedExtensions] = {
		".jpg",
		".jpeg",
		".JPEG",
		".JPG",
		".png",
		".PNG",
		".tif",
		".TIF",
		".webp",
		".WEBP",
	};

	SDLImageImporter::SDLImageImporter() : m_InitializedFlags(0)
	{
	}

	SDLImageImporter::~SDLImageImporter()
	{
	}

	std::string_view SDLImageImporter::Name() const
	{
		return "SDLImage Importer";
	}

	bool SDLImageImporter::SupportsExtension(const std::filesystem::path& extension) const
	{
		for (size_t i = 0; i < NumSupportedExtensions; ++i)
		{
			if (extension.compare(SupportedExtensions[i]) != 0) continue;
			return true;
		}
		return false;
	}

	ImportedResource SDLImageImporter::LoadResource(const std::filesystem::path& path, void* userData) const
	{
		SDL_Surface* pSDLImage = IMG_Load(path.string().data());

		if (pSDLImage == NULL)
		{
			const char* error = SDL_GetError();
			std::stringstream str;
			str << "SDL Could not load image " << path << " SDL Error: " << error;
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogWarning(str.str());
			return nullptr;
		}

		return Process(path, pSDLImage);
	}

	ImportedResource SDLImageImporter::LoadResource(void* data, size_t dataSize, void* userData) const
	{
		SDL_RWops* rwops = SDL_RWFromMem(data, dataSize);
		SDL_Surface* pSDLImage = IMG_Load_RW(rwops, 1);

		if (pSDLImage == NULL)
		{
			const char* error = SDL_GetError();
			std::stringstream str;
			str << "SDL Could not load image from memory, SDL Error: " << error;
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogWarning(str.str());
			return nullptr;
		}

		return Process("", pSDLImage);
	}

	void SDLImageImporter::Initialize()
	{
		// Initialize for all available extensions
		m_InitializedFlags = IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP);
		if (m_InitializedFlags == 0)
		{
			const char* error = SDL_GetError();
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogFatalError("Could not initialize SDL_image, SDL Error: " + std::string(error));
		}
	}

	void SDLImageImporter::Cleanup()
	{
		IMG_Quit();
	}

	ImportedResource SDLImageImporter::Process(const std::filesystem::path& path, SDL_Surface* pSDLImage) const
	{
		const uint32_t width = static_cast<uint32_t>(pSDLImage->w);
		const uint32_t height = static_cast<uint32_t>(pSDLImage->h);

		const size_t numPixels = width * height;

		PixelFormat pixelFormat = Glory::PixelFormat::PF_R8G8B8Srgb;
		PixelFormat internalFormat = Glory::PixelFormat::PF_RGB;
		uint8_t bytesPerPixel = pSDLImage->format->BytesPerPixel;

		switch (bytesPerPixel)
		{
		case 3:
			if (pSDLImage->format->Rmask == 0x000000ff)
			{
				pixelFormat = PixelFormat::PF_RGB;
				internalFormat = PixelFormat::PF_R8G8B8Srgb;
			}
			else
			{
				pixelFormat = PixelFormat::PF_RGB;
				internalFormat = PixelFormat::PF_B8G8R8Srgb;
			}
			break;
		case 4:
			if (pSDLImage->format->Rmask == 0x000000ff)
			{
				pixelFormat = PixelFormat::PF_RGBA;
				internalFormat = PixelFormat::PF_R8G8B8A8Srgb;
			}
			else
			{
				pixelFormat = PixelFormat::PF_RGBA;
				internalFormat = PixelFormat::PF_B8G8R8A8Srgb;
			}
			break;
		default:
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogError("SDLImageImporter::LoadResource > unknow pixel format, BytesPerPixel: " + std::to_string(pSDLImage->format->BytesPerPixel) + " Use 32 bit or 24 bit images.\n");
			SDL_FreeSurface(pSDLImage);
			return nullptr;
		}

		char* data = new char[bytesPerPixel * numPixels];
		std::memcpy(data, pSDLImage->pixels, bytesPerPixel * numPixels);

		ImageData* pData = new ImageData(width, height, internalFormat, pixelFormat, bytesPerPixel, std::move(data), bytesPerPixel*numPixels);
		ImportedResource importedResource{ path, pData };

		TextureData* pDefualtTexture = new TextureData(pData);
		importedResource.AddChild(pDefualtTexture, "Default");

		SDL_FreeSurface(pSDLImage);
		return importedResource;
	}
}

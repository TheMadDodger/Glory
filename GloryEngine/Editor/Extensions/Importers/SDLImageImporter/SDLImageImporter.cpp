#include "SDLImageImporter.h"

#include <EditorApplication.h>
#include <SDL2/SDL_image.h>
#include <sstream>
#include <Debug.h>
#include <TextureData.h>
#include <bitset>

namespace Glory::Editor
{
	constexpr size_t NumSupportedExtensions = 12;
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
		".tga",
		".TGA"
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

		const size_t numPixels = width*height;

		PixelFormat pixelFormat = Glory::PixelFormat::PF_R8G8B8Unorm;
		PixelFormat internalFormat = Glory::PixelFormat::PF_RGB;

		const uint8_t bytesPerPixel = pSDLImage->format->BytesPerPixel;

		const size_t redBits = std::bitset<32>(pSDLImage->format->Rmask).count();
		const size_t greenBits = std::bitset<32>(pSDLImage->format->Bmask).count();
		const size_t blueBits = std::bitset<32>(pSDLImage->format->Gmask).count();
		const size_t alphaBits = std::bitset<32>(pSDLImage->format->Amask).count();

		Debug& debug = EditorApplication::GetInstance()->GetEngine()->GetDebug();
		if (redBits > 8 || greenBits > 8 || blueBits > 8 || alphaBits > 8)
		{
			debug.LogError("SDLImageImporter::LoadResource > Failed to import image, each channel of a pixel can't have more than 8 bits!");
			SDL_FreeSurface(pSDLImage);
			return nullptr;
		}

		const size_t numChannels = size_t(redBits > 0) + size_t(greenBits > 0) +
			size_t(blueBits > 0) + size_t(alphaBits > 0);

		const size_t finalChannelCount = numChannels > 2 ? 4 : numChannels;
		const size_t finalBytesPerPixel = finalChannelCount*sizeof(char);

		switch (finalChannelCount)
		{
		case 1:
			pixelFormat = PixelFormat::PF_R;
			internalFormat = PixelFormat::PF_R8Unorm;
			break;
		case 2:
			pixelFormat = PixelFormat::PF_RG;
			internalFormat = PixelFormat::PF_R8G8Unorm;
			break;
		case 4:
			pixelFormat = PixelFormat::PF_RGBA;
			internalFormat = PixelFormat::PF_R8G8B8A8Unorm;
			break;
		default:
			debug.LogError("SDLImageImporter::LoadResource > Failed to import image, format not supported!");
			SDL_FreeSurface(pSDLImage);
			return nullptr;
		}

		std::function<void(char*, char*, size_t, size_t, size_t)> readChannel =
		[](char* inPixel, char* outPixel, size_t startingOffset, size_t bits, size_t defaultValue) {
			if (!bits)
			{
				if (defaultValue) *outPixel = defaultValue;
				return;
			}
			const size_t offset = startingOffset/8;
			char* channelStart = inPixel + offset;
			const size_t bitOffset = startingOffset - offset*8;
			const size_t mask = ((1 << bits) - 1) << bitOffset;
			*outPixel = (*channelStart) & mask;
		};

		char* pixels = (char*)pSDLImage->pixels;
		char* convertedPixels = new char[finalBytesPerPixel*numPixels];
		for (size_t i = 0; i < numPixels; ++i)
		{
			char* pixelStart = pixels + bytesPerPixel*i;
			char* convertedPixel = convertedPixels + finalBytesPerPixel*i;
			readChannel(pixelStart, convertedPixel, pSDLImage->format->Rshift, redBits, 0);
			readChannel(pixelStart, convertedPixel + sizeof(char), pSDLImage->format->Gshift, greenBits, 0);
			readChannel(pixelStart, convertedPixel + sizeof(char)*2, pSDLImage->format->Bshift, blueBits, 0);
			if (finalChannelCount == 4)
				readChannel(pixelStart, convertedPixel + sizeof(char)*3, pSDLImage->format->Ashift, alphaBits, 255);
		}

		ImageData* pData = new ImageData(width, height, internalFormat, pixelFormat, finalBytesPerPixel,
			std::move(convertedPixels), finalBytesPerPixel*numPixels);
		ImportedResource importedResource{ path, pData };

		TextureData* pDefualtTexture = new TextureData(pData);
		importedResource.AddChild(pDefualtTexture, "Default");

		SDL_FreeSurface(pSDLImage);
		return importedResource;
	}
}

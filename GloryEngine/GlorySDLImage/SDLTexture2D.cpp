#include "SDLTexture2D.h"
#include <Debug.h>

namespace Glory
{
	SDLImageData::SDLImageData() : m_pSurface(nullptr)
	{
	}

	SDLImageData::~SDLImageData()
	{
		SDL_FreeSurface(m_pSurface);
		m_pSurface = nullptr;
	}

	const void* SDLImageData::GetPixels()
	{
		return m_pSurface->pixels;
	}

	void SDLImageData::BuildTexture()
	{
		m_Width = static_cast<uint32_t>(m_pSurface->w);
		m_Height = static_cast<uint32_t>(m_pSurface->h);

		m_PixelFormat = Glory::PixelFormat::PF_R8G8B8Srgb;
		m_BytesPerPixel = m_pSurface->format->BytesPerPixel;

		switch (m_BytesPerPixel)
		{
		case 3:
			if (m_pSurface->format->Rmask == 0x000000ff)
			{
				m_PixelFormat = Glory::PixelFormat::PF_R8G8B8Srgb;
			}
			else
			{
				m_PixelFormat = Glory::PixelFormat::PF_B8G8R8Srgb;
			}
			break;
		case 4:
			if (m_pSurface->format->Rmask == 0x000000ff)
			{
				m_PixelFormat = Glory::PixelFormat::PF_R8G8B8A8Srgb;
			}
			else
			{
				m_PixelFormat = Glory::PixelFormat::PF_B8G8R8A8Srgb;
			}
			break;
		default:
			Debug::LogError("Texture::CreateFromSurface, unknow pixel format, BytesPerPixel: " + std::to_string(m_pSurface->format->BytesPerPixel) + " Use 32 bit or 24 bit images.\n");
			return;
		}
	}
}

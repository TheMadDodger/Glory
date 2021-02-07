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

	const float* SDLImageData::GetPixels()
	{
		return (const float*)m_pSurface->pixels;
	}

	void SDLImageData::BuildTexture()
	{
		m_Width = static_cast<uint32_t>(m_pSurface->w);
		m_Height = static_cast<uint32_t>(m_pSurface->h);

		m_PixelFormat = Glory::PixelFormat::PF_RGB;
		m_BytesPerPixel = m_pSurface->format->BytesPerPixel;

		switch (m_BytesPerPixel)
		{
		case 3:
			if (m_pSurface->format->Rmask == 0x000000ff)
			{
				m_PixelFormat = Glory::PixelFormat::PF_RGB;
			}
			else
			{
				m_PixelFormat = Glory::PixelFormat::PF_BGR;
			}
			break;
		case 4:
			if (m_pSurface->format->Rmask == 0x000000ff)
			{
				m_PixelFormat = Glory::PixelFormat::PF_RGBA;
			}
			else
			{
				m_PixelFormat = Glory::PixelFormat::PF_BGRA;
			}
			break;
		default:
			Debug::LogError("Texture::CreateFromSurface, unknow pixel format, BytesPerPixel: " + std::to_string(m_pSurface->format->BytesPerPixel) + " Use 32 bit or 24 bit images.\n");
			return;
		}
	}
}

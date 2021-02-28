#include "ImageData.h"

namespace Glory
{
	ImageData::ImageData() : m_Width(0), m_Height(0), m_PixelFormat(PixelFormat::PF_R8G8B8Srgb), m_BytesPerPixel(0)
	{
	}

	ImageData::~ImageData()
	{
	}

	uint32_t ImageData::GetWidth()
	{
		return m_Width;
	}

	uint32_t ImageData::GetHeight()
	{
		return m_Height;
	}

	uint8_t ImageData::GetBytesPerPixel()
	{
		return m_BytesPerPixel;
	}

	uint32_t ImageData::GetByteSize()
	{
		return m_Width * m_Height * m_BytesPerPixel;
	}

	const PixelFormat& ImageData::GetFormat()
	{
		return m_PixelFormat;
	}
}

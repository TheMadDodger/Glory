#include "ImageData.h"

namespace Glory
{
	ImageData::ImageData(uint32_t w, uint32_t h, PixelFormat internalFormat, PixelFormat format, uint8_t bytesPerPixel, const char*&& pPixels, size_t dataSize)
		: m_Width(w), m_Height(h), m_InternalFormat(internalFormat), m_PixelFormat(format), m_BytesPerPixel(bytesPerPixel), m_pPixels(std::move(pPixels)), m_DataSize(dataSize) {}

	ImageData::ImageData() : m_Width(0), m_Height(0), m_InternalFormat(PixelFormat::PF_RGB), m_PixelFormat(PixelFormat::PF_R8G8B8Srgb), m_BytesPerPixel(0), m_pPixels(nullptr), m_DataSize(0)
	{
		APPEND_TYPE(ImageData);
	}

	ImageData::~ImageData()
	{
	}

	uint32_t ImageData::GetWidth() const
	{
		return m_Width;
	}

	uint32_t ImageData::GetHeight() const
	{
		return m_Height;
	}

	uint8_t ImageData::GetBytesPerPixel() const
	{
		return m_BytesPerPixel;
	}

	uint32_t ImageData::GetByteSize() const
	{
		return m_Width * m_Height * m_BytesPerPixel;
	}

	const void* ImageData::GetPixels() const
	{
		return m_pPixels;
	}

	size_t ImageData::DataSize() const
	{
		return m_DataSize;
	}

	const PixelFormat& ImageData::GetFormat() const
	{
		return m_PixelFormat;
	}

	const PixelFormat& ImageData::GetInternalFormat() const
	{
		return m_InternalFormat;
	}

	void ImageData::BuildTexture() {}
}

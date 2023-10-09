#include "ImageData.h"
#include "BinaryStream.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>

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

	void ImageData::Serialize(BinaryStream& container) const
	{
		container.Write(m_Width);
		container.Write(m_Height);
		container.Write(m_InternalFormat);
		container.Write(m_PixelFormat);
		container.Write(m_BytesPerPixel);

		const int channels = m_InternalFormat == PixelFormat::PF_RGBA ? 4 : 3;
		/* TODO: Use libpng to compress this even further */
		stbi_write_png_to_func([](void* context, void* data, int size)
		{
			BinaryStream* container = (BinaryStream*)context;
			container->Write(size);
			container->Write(data, size_t(size));
		}, &container, int(m_Width), int(m_Height), int(channels), m_pPixels, int(m_BytesPerPixel * m_Width));
	}

	void ImageData::Deserialize(BinaryStream& container) const
	{

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

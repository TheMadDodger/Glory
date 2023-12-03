#include "ImageData.h"
#include "BinaryStream.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>

namespace Glory
{
	ImageData::ImageData(uint32_t w, uint32_t h, PixelFormat internalFormat, PixelFormat format, uint8_t bytesPerPixel, char*&& pPixels, size_t dataSize, bool compressed)
		: m_Header{ w, h, internalFormat, format, bytesPerPixel, dataSize, compressed }, m_pPixels(std::move(pPixels)) {}

	ImageData::ImageData() : m_Header{}, m_pPixels(nullptr)
	{
		APPEND_TYPE(ImageData);
	}

	ImageData::~ImageData()
	{
		delete m_pPixels;
		m_pPixels = nullptr;
	}

	uint32_t ImageData::GetWidth() const
	{
		return m_Header.m_Width;
	}

	uint32_t ImageData::GetHeight() const
	{
		return m_Header.m_Height;
	}

	uint8_t ImageData::GetBytesPerPixel() const
	{
		return m_Header.m_BytesPerPixel;
	}

	uint32_t ImageData::GetByteSize() const
	{
		return m_Header.m_Width * m_Header.m_Height * m_Header.m_BytesPerPixel;
	}

	const void* ImageData::GetPixels() const
	{
		return m_pPixels;
	}

	size_t ImageData::DataSize() const
	{
		return m_Header.m_DataSize;
	}

	void ImageData::Serialize(BinaryStream& container) const
	{
		const int channels = m_Header.m_InternalFormat == PixelFormat::PF_RGBA ? 4 : 3;
		if (m_Header.m_Compressed)
		{
			/* No need to compress if it was already compressed by the importer */
			container.Write(m_Header).Write(m_pPixels, m_Header.m_DataSize);
			return;
		}

		/* TODO: Use libpng to compress this even further */
		struct ContextData
		{
			BinaryStream* stream;
			const ImageData* image;
		};
		ContextData data{ &container, this };

		stbi_write_png_to_func([](void* context, void* data, int size)
		{
			ContextData* contextData = (ContextData*)context;
			/* Overwrite the size */
			Header h = contextData->image->m_Header;
			h.m_DataSize = size;
			contextData->stream->Write(h);
			contextData->stream->Write(size);
			contextData->stream->Write(data, size_t(size));
		}, &data, int(m_Header.m_Width), int(m_Header.m_Height), int(channels), m_pPixels, int(m_Header.m_BytesPerPixel * m_Header.m_Width));
	}

	void ImageData::Deserialize(BinaryStream& container)
	{
		container.Read(m_Header);

		if (!m_Header.m_Compressed)
		{
			/* Image is not compressed or was compressed by the importer */
			m_pPixels = new char[m_Header.m_DataSize];
			container.Read(m_pPixels, m_Header.m_DataSize);
			return;
		}

		//stbi__context context;
		//stbi__png_load(&context, );
	}

	const PixelFormat& ImageData::GetFormat() const
	{
		return m_Header.m_PixelFormat;
	}

	const PixelFormat& ImageData::GetInternalFormat() const
	{
		return m_Header.m_InternalFormat;
	}

	void ImageData::BuildTexture() {}
}

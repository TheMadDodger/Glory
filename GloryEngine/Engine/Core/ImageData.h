#pragma once
#include "Resource.h"
#include <cstdint>
#include "GraphicsEnums.h"

namespace Glory
{
    class ImageData : public Resource
    {
    public:
        ImageData(uint32_t w, uint32_t h, PixelFormat internalFormat, PixelFormat format, uint8_t bytesPerPixel, const char*&& pPixels, size_t dataSize);
        ImageData();
        virtual ~ImageData();

        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
        const PixelFormat& GetFormat() const;
        const PixelFormat& GetInternalFormat() const;
        uint8_t GetBytesPerPixel() const;
        uint32_t GetByteSize() const;
        virtual const void* GetPixels() const;
        virtual size_t DataSize() const;

    protected:
        uint32_t m_Width;
        uint32_t m_Height;
        PixelFormat m_InternalFormat;
        PixelFormat m_PixelFormat;
        uint8_t m_BytesPerPixel;

        const char* m_pPixels;
        size_t m_DataSize;

        virtual void BuildTexture();

    private:
        friend class ImageLoaderModule;
    };
}

#pragma once
#include "Resource.h"
#include <cstdint>
#include "GraphicsEnums.h"

namespace Glory
{
    class ImageData : public Resource
    {
    public:
        ImageData();
        virtual ~ImageData();

        uint32_t GetWidth();
        uint32_t GetHeight();
        const PixelFormat& GetFormat();
        const PixelFormat& GetInternalFormat();
        uint8_t GetBytesPerPixel();
        uint32_t GetByteSize();
        virtual const void* GetPixels();

    protected:
        uint32_t m_Width;
        uint32_t m_Height;
        PixelFormat m_InternalFormat;
        PixelFormat m_PixelFormat;
        uint8_t m_BytesPerPixel;

        virtual void BuildTexture();

    private:
        friend class ImageLoaderModule;
    };
}

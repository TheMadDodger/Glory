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
        uint8_t GetBytesPerPixel();
        uint32_t GetByteSize();
        virtual const float* GetPixels() = 0;

    protected:
        uint32_t m_Width;
        uint32_t m_Height;
        PixelFormat m_PixelFormat;
        uint8_t m_BytesPerPixel;

        virtual void BuildTexture() = 0;

    private:
        friend class ImageLoaderModule;
    };
}

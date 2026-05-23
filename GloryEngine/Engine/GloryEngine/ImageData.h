#pragma once
#include "Resource.h"
#include "GraphicsEnums.h"

#include <engine_visibility.h>

#include <cstdint>

namespace Glory
{
    class ImageData : public Resource
    {
    public:
        struct Header
        {
            uint32_t m_Width;
            uint32_t m_Height;
            PixelFormat m_InternalFormat;
            PixelFormat m_PixelFormat;
            uint8_t m_BytesPerPixel;
            size_t m_DataSize;
            bool m_Compressed;
            DataType m_DataType;
        };

    public:
        GLORY_ENGINE_API ImageData();
        GLORY_ENGINE_API ImageData(uint32_t w, uint32_t h, PixelFormat internalFormat, PixelFormat format, uint8_t bytesPerPixel, char*&& pPixels, size_t dataSize, bool compressed=false, DataType dataType=DataType::DT_UByte);
        GLORY_ENGINE_API ImageData(ImageData&& other) noexcept;
        GLORY_ENGINE_API ImageData& operator=(ImageData&& other) noexcept;
        GLORY_ENGINE_API virtual ~ImageData();

        GLORY_ENGINE_API uint32_t GetWidth() const;
        GLORY_ENGINE_API uint32_t GetHeight() const;
        GLORY_ENGINE_API const PixelFormat& GetFormat() const;
        GLORY_ENGINE_API const PixelFormat& GetInternalFormat() const;
        GLORY_ENGINE_API uint8_t GetBytesPerPixel() const;
        GLORY_ENGINE_API uint32_t GetByteSize() const;
        GLORY_ENGINE_API virtual const void* GetPixels() const;
        GLORY_ENGINE_API virtual void* GetPixels();
        GLORY_ENGINE_API virtual size_t DataSize() const;
        GLORY_ENGINE_API virtual DataType GetDataType() const;

        GLORY_ENGINE_API void SetPixels(char*&& pPixels, size_t dataSize);

        GLORY_ENGINE_API void Serialize(Utils::BinaryStream& container) const override;
        GLORY_ENGINE_API void Deserialize(Utils::BinaryStream& container) override;

    protected:
        Header m_Header;
        char* m_pPixels;

        virtual void BuildTexture();

        virtual void References(IEngine*, std::vector<UUID>&) const override {}

    private:
        friend class ImageLoaderModule;
    };
}

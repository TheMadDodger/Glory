#pragma once
#include <Texture.h>
#include <vulkan/vulkan.hpp>

namespace Glory
{
    class VulkanTexture : public Texture
    {
    public:
        VulkanTexture(TextureCreateInfo&& textureInfo);
        virtual ~VulkanTexture();

        GLORY_API const vk::Image GetTextureImage() const;
        GLORY_API const vk::DeviceMemory GetTextureImageMemory() const;
        GLORY_API const vk::ImageView GetTextureImageView() const;
        GLORY_API const vk::Sampler GetTextureSampler();

    private:
        GLORY_API virtual void Create(TextureData* pTextureData) override;
        GLORY_API virtual void Create(CubemapData* pCubemapData) override;
        GLORY_API virtual void Create(const void* pixels = nullptr) override;
        GLORY_API virtual void CopyFromBuffer(Buffer* pBuffer, int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth) override;

    private:
        vk::Image m_TextureImage;
        vk::DeviceMemory m_TextureImageMemory;
        vk::ImageView m_TextureImageView;
    };
}

#pragma once
#include <Texture.h>
#include <vulkan/vulkan.hpp>
#include "VulkanImageSampler.h"

namespace Glory
{
    class VulkanTexture : public Texture
    {
    public:
        VulkanTexture(uint32_t width, uint32_t height, const PixelFormat& format, const PixelFormat& internalFormat, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings = SamplerSettings());
        virtual ~VulkanTexture();

        virtual void Create(ImageData* pImage) override;
        virtual void Create() override;
        void CopyFromBuffer(Buffer* pBuffer, int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth) override;

        const vk::Image GetTextureImage() const;
        const vk::DeviceMemory GetTextureImageMemory() const;
        const vk::ImageView GetTextureImageView() const;
        const vk::Sampler GetTextureSampler();

    private:
        vk::Image m_TextureImage;
        vk::DeviceMemory m_TextureImageMemory;
        vk::ImageView m_TextureImageView;
        VulkanImageSampler* m_pSampler;
    };
}

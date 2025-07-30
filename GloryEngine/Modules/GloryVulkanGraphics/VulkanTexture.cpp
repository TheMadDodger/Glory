#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "VulkanGraphicsModule.h"
#include "Device.h"
#include "VulkanBuffer.h"
#include "VulkanStructsConverter.h"

#include "VulkanDevice.h"

#include <Engine.h>

namespace Glory
{
	VulkanTexture::VulkanTexture(TextureCreateInfo&& textureInfo)
        : Texture(std::move(textureInfo))
	{
	}

	VulkanTexture::~VulkanTexture()
	{
        VulkanGraphicsModule* pGraphics = m_pOwner->GetEngine()->GetMainModule<VulkanGraphicsModule>();
        VulkanDeviceManager& deviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = deviceManager.GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();
        
        deviceData.LogicalDevice.destroyImageView(m_TextureImageView, nullptr);
        deviceData.LogicalDevice.destroyImage(m_TextureImage, nullptr);
        deviceData.LogicalDevice.freeMemory(m_TextureImageMemory, nullptr);
	}

    void VulkanTexture::CopyFromBuffer(Buffer* pBuffer, int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth)
    {
        VulkanGraphicsModule* pGraphics = m_pOwner->GetEngine()->GetMainModule<VulkanGraphicsModule>();
        VulkanBuffer* pVulkanBuffer = (VulkanBuffer*)pBuffer;
        vk::CommandBuffer commandBuffer = pGraphics->BeginSingleTimeCommands();

        vk::BufferImageCopy copyRegion = vk::BufferImageCopy();
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;

        copyRegion.imageSubresource.aspectMask = VKConverter::GetVulkanImageAspectFlags(m_TextureInfo.m_ImageAspectFlags);
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;

        copyRegion.imageOffset = vk::Offset3D(offsetX, offsetY, offsetZ);
        copyRegion.imageExtent = vk::Extent3D(width, height, depth);

        commandBuffer.copyBufferToImage(pVulkanBuffer->GetBuffer(), m_TextureImage, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);

        pGraphics->EndSingleTimeCommands(commandBuffer);
    }

    const vk::Image VulkanTexture::GetTextureImage() const
    {
        return m_TextureImage;
    }

    const vk::DeviceMemory VulkanTexture::GetTextureImageMemory() const
    {
        return m_TextureImageMemory;
    }

    const vk::ImageView VulkanTexture::GetTextureImageView() const
    {
        return m_TextureImageView;
    }

    const vk::Sampler VulkanTexture::GetTextureSampler()
    {
        VulkanGraphicsModule* pGraphics = m_pOwner->GetEngine()->GetMainModule<VulkanGraphicsModule>();
        return pGraphics->GetSampler(m_TextureInfo.m_SamplerSettings);
    }

    void VulkanTexture::Create(TextureData* pTextureData)
    {
        ImageData* pImage = pTextureData->GetImageData(&m_pOwner->GetEngine()->GetAssetManager());
        VulkanGraphicsModule* pGraphics = m_pOwner->GetEngine()->GetMainModule<VulkanGraphicsModule>();
        GPUResourceManager* pResources = pGraphics->GetResourceManager();
        VulkanDeviceManager& deviceManager = pGraphics->GetDeviceManager();
        VulkanDevice* pDevice = static_cast<VulkanDevice*>(m_pOwner->GetEngine()->ActiveGraphicsDevice());

        uint32_t imageSize = pImage->GetByteSize();

        //vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        Buffer* pTextureStagingBuffer = pResources->CreateBuffer(imageSize, BufferBindingTarget::B_COPY_READ, MemoryUsage::MU_DYNAMIC_COPY, 0);//new VulkanBuffer(imageSize, (uint32_t)vk::BufferUsageFlagBits::eTransferSrc, (uint32_t)memoryFlags);
        pTextureStagingBuffer->Assign(pImage->GetPixels());

        vk::Format format = VKConverter::GetVulkanFormat(m_TextureInfo.m_InternalFormat);

        vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo();
        vk::ImageType imageType = VKConverter::GetVulkanImageType(m_TextureInfo.m_ImageType);
        imageInfo.imageType = imageType;
        imageInfo.extent.width = pImage->GetWidth();
        imageInfo.extent.height = pImage->GetHeight();
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = format == vk::Format::eR8G8B8A8Srgb ? vk::ImageTiling::eOptimal : (vk::ImageTiling)0;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage = VKConverter::GetVulkanImageUsageFlags(m_TextureInfo.m_ImageAspectFlags);
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
        imageInfo.flags = (vk::ImageCreateFlags)0;

        if (pDevice->LogicalDevice().createImage(&imageInfo, nullptr, &m_TextureImage) != vk::Result::eSuccess)
            throw std::runtime_error("Failed to create image!");

        vk::MemoryRequirements memRequirements;
        pDevice->LogicalDevice().getImageMemoryRequirements(m_TextureImage, &memRequirements);

        uint32_t typeFilter = memRequirements.memoryTypeBits;
        vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
        uint32_t memoryIndex = pDevice->GetSupportedMemoryIndex(typeFilter, properties);

        vk::MemoryAllocateInfo imageAllocInfo = vk::MemoryAllocateInfo();
        imageAllocInfo.allocationSize = memRequirements.size;
        imageAllocInfo.memoryTypeIndex = memoryIndex;

        if (pDevice->LogicalDevice().allocateMemory(&imageAllocInfo, nullptr, &m_TextureImageMemory) != vk::Result::eSuccess)
            throw std::runtime_error("Failed to allocate image memory!");
        pDevice->LogicalDevice().bindImageMemory(m_TextureImage, m_TextureImageMemory, 0);

        // Transition image layout
        pGraphics->TransitionImageLayout(m_TextureImage, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

        // Copy buffer to image
        Texture::CopyFromBuffer(pTextureStagingBuffer);

        // Transtion layout again so it can be sampled
        pGraphics->TransitionImageLayout(m_TextureImage, format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        pResources->Free(pTextureStagingBuffer);

        // Create texture image view
        vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo();
        viewInfo.image = m_TextureImage;
        viewInfo.viewType = VKConverter::GetVulkanImageViewType(m_TextureInfo.m_ImageType);
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VKConverter::GetVulkanImageAspectFlags(m_TextureInfo.m_ImageAspectFlags);
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (format == vk::Format::eR8G8B8Unorm)
            viewInfo.components.a = vk::ComponentSwizzle::eOne;

        if (pDevice->LogicalDevice().createImageView(&viewInfo, nullptr, &m_TextureImageView) != vk::Result::eSuccess)
            throw std::runtime_error("failed to create texture image view!");
    }

    void VulkanTexture::Create(CubemapData* pCubemapData)
    {
        throw new std::exception("VulkanBuffer::BindForDraw() not yet implemented!");
    }

    void VulkanTexture::Create(const void* pixels)
    {
        VulkanGraphicsModule* pGraphics = m_pOwner->GetEngine()->GetMainModule<VulkanGraphicsModule>();
        GPUResourceManager* pResources = pGraphics->GetResourceManager();
        VulkanDeviceManager& deviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = deviceManager.GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        vk::Format format = VKConverter::GetVulkanFormat(m_TextureInfo.m_PixelFormat); //vk::Format::eR8G8B8A8Srgb;
        vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo();
        vk::ImageType imageType = VKConverter::GetVulkanImageType(m_TextureInfo.m_ImageType);
        imageInfo.imageType = imageType;
        imageInfo.extent.width = m_TextureInfo.m_Width;
        imageInfo.extent.height = m_TextureInfo.m_Height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = format == vk::Format::eR8G8B8A8Srgb ? vk::ImageTiling::eOptimal : (vk::ImageTiling)0;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage = VKConverter::GetVulkanImageUsageFlags(m_TextureInfo.m_ImageAspectFlags);
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
        imageInfo.flags = (vk::ImageCreateFlags)0;

        if (deviceData.LogicalDevice.createImage(&imageInfo, nullptr, &m_TextureImage) != vk::Result::eSuccess)
            throw std::runtime_error("Failed to create image!");

        vk::MemoryRequirements memRequirements;
        deviceData.LogicalDevice.getImageMemoryRequirements(m_TextureImage, &memRequirements);

        uint32_t typeFilter = memRequirements.memoryTypeBits;
        vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
        uint32_t memoryIndex = pDevice->GetSupportedMemoryIndex(typeFilter, properties);

        vk::MemoryAllocateInfo imageAllocInfo = vk::MemoryAllocateInfo();
        imageAllocInfo.allocationSize = memRequirements.size;
        imageAllocInfo.memoryTypeIndex = memoryIndex;

        if (deviceData.LogicalDevice.allocateMemory(&imageAllocInfo, nullptr, &m_TextureImageMemory) != vk::Result::eSuccess)
            throw std::runtime_error("Failed to allocate image memory!");
        deviceData.LogicalDevice.bindImageMemory(m_TextureImage, m_TextureImageMemory, 0);

        // Copy buffer to image
        if (pixels)
        {
            // Transition image layout
            pGraphics->TransitionImageLayout(m_TextureImage, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

            vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
            Buffer* pTextureStagingBuffer = pResources->CreateBuffer(uint32_t(memRequirements.size), BufferBindingTarget::B_COPY_READ, MemoryUsage::MU_DYNAMIC_COPY, 0);//new VulkanBuffer(imageSize, (uint32_t)vk::BufferUsageFlagBits::eTransferSrc, (uint32_t)memoryFlags);
            pTextureStagingBuffer->Assign(pixels);
            Texture::CopyFromBuffer(pTextureStagingBuffer);

            // Transtion layout again so it can be sampled
            pGraphics->TransitionImageLayout(m_TextureImage, format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

            pResources->Free(pTextureStagingBuffer);
        }

        // Create texture image view
        vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo();
        viewInfo.image = m_TextureImage;
        viewInfo.viewType = VKConverter::GetVulkanImageViewType(m_TextureInfo.m_ImageType);
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VKConverter::GetVulkanImageAspectFlags(m_TextureInfo.m_ImageAspectFlags);
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (deviceData.LogicalDevice.createImageView(&viewInfo, nullptr, &m_TextureImageView) != vk::Result::eSuccess)
            throw std::runtime_error("failed to create texture image view!");
    }
}

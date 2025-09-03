#include "VulkanBuffer.h"
#include "VulkanGraphicsModule.h"
#include "Device.h"
#include "VulkanStructsConverter.h"

#include "VulkanDevice.h"

#include <Engine.h>

namespace Glory
{
    VulkanBuffer::VulkanBuffer(uint32_t bufferSize, BufferBindingTarget usageFlag, MemoryUsage memoryFlags, uint32_t bindIndex) :
        Buffer(bufferSize, usageFlag, memoryFlags, bindIndex)
	{
	}

    VulkanBuffer::~VulkanBuffer()
	{
        VulkanDevice* pDevice = static_cast<VulkanDevice*>(m_pOwner->GetEngine()->ActiveGraphicsDevice());
        pDevice->LogicalDevice().destroyBuffer(m_Buffer);
        pDevice->LogicalDevice().freeMemory(m_BufferMemory);
	}

    void VulkanBuffer::CreateBuffer()
	{
        m_BufferCreateInfo = vk::BufferCreateInfo();
        m_BufferCreateInfo.size = (vk::DeviceSize)m_BufferSize;
        m_BufferCreateInfo.usage = VKConverter::ToBufferUsageFlags(m_UsageFlag);
        m_BufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

        VulkanGraphicsModule* pGraphics = m_pOwner->GetEngine()->GetMainModule<VulkanGraphicsModule>();
        VulkanDeviceManager& pDeviceManager = pGraphics->GetDeviceManager();
        VulkanDevice* pDevice = static_cast<VulkanDevice*>(m_pOwner->GetEngine()->ActiveGraphicsDevice());

        vk::Result result = pDevice->LogicalDevice().createBuffer(&m_BufferCreateInfo, nullptr, &m_Buffer);
        if (result != vk::Result::eSuccess)
            throw std::runtime_error("failed to create vertex buffer!");

        vk::MemoryRequirements memRequirements;
        pDevice->LogicalDevice().getBufferMemoryRequirements(m_Buffer, &memRequirements);

        uint32_t typeFilter = memRequirements.memoryTypeBits;
        vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;//(vk::MemoryPropertyFlagBits)m_MemoryFlags;
        uint32_t memoryIndex = pDevice->GetSupportedMemoryIndex(typeFilter, properties);

        // Allocate device memory
        vk::MemoryAllocateInfo allocateInfo = vk::MemoryAllocateInfo();
        allocateInfo.allocationSize = memRequirements.size;
        allocateInfo.memoryTypeIndex = memoryIndex;

        result = pDevice->LogicalDevice().allocateMemory(&allocateInfo, nullptr, &m_BufferMemory);
        if (result != vk::Result::eSuccess)
            throw std::runtime_error("Failed to allocate vertex buffer memory!");

        pDevice->LogicalDevice().bindBufferMemory(m_Buffer, m_BufferMemory, 0);
	}

    void VulkanBuffer::Assign(const void* data)
    {
        if (!data) return;

        VulkanGraphicsModule* pGraphics = m_pOwner->GetEngine()->GetMainModule<VulkanGraphicsModule>();
        VulkanDeviceManager& pDeviceManager = pGraphics->GetDeviceManager();
        VulkanDevice* pDevice = static_cast<VulkanDevice*>(m_pOwner->GetEngine()->ActiveGraphicsDevice());

        void* dstData;
        vk::Result result = pDevice->LogicalDevice().mapMemory(m_BufferMemory, (vk::DeviceSize)0, (vk::DeviceSize)m_BufferCreateInfo.size, (vk::MemoryMapFlags)0, &dstData);
        if (result != vk::Result::eSuccess)
            throw std::runtime_error("Failed to map memory!");
        memcpy(dstData, data, (size_t)m_BufferCreateInfo.size);
        pDevice->LogicalDevice().unmapMemory(m_BufferMemory);
    }

    void VulkanBuffer::Assign(const void* data, uint32_t size)
    {
        throw new std::exception("VulkanBuffer::Assign() not yet implemented!");
    }

    void VulkanBuffer::Assign(const void* data, uint32_t offset, uint32_t size)
    {
        VulkanGraphicsModule* pGraphics = m_pOwner->GetEngine()->GetMainModule<VulkanGraphicsModule>();
        VulkanDeviceManager& deviceManager = pGraphics->GetDeviceManager();
        VulkanDevice* pDevice = static_cast<VulkanDevice*>(m_pOwner->GetEngine()->ActiveGraphicsDevice());

        void* dstData;
        vk::Result result = pDevice->LogicalDevice().mapMemory(m_BufferMemory, (vk::DeviceSize)offset, (vk::DeviceSize)size, (vk::MemoryMapFlags)0, &dstData);
        if (result != vk::Result::eSuccess)
            throw std::runtime_error("Failed to map memory!");
        memcpy(dstData, data, (size_t)m_BufferCreateInfo.size);
        pDevice->LogicalDevice().unmapMemory(m_BufferMemory);
    }

    void VulkanBuffer::CopyFrom(Buffer* source, uint32_t size)
    {
        // Memory transfer operations are executed using command buffers,
        // just like drawing commands. Therefore we must first allocate a temporary command buffer.
        // You may wish to create a separate command pool for these kinds of short-lived buffers,
        // because the implementation may be able to apply memory allocation optimizations.
        // You should use the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag
        // during command pool generation in that case.

        VulkanGraphicsModule* pGraphics = m_pOwner->GetEngine()->GetMainModule<VulkanGraphicsModule>();
        vk::CommandBuffer commandBuffer = pGraphics->BeginSingleTimeCommands();

        vk::BufferCopy copyRegion = vk::BufferCopy();
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        commandBuffer.copyBuffer(((VulkanBuffer*)source)->m_Buffer, m_Buffer, 1, &copyRegion);

        pGraphics->EndSingleTimeCommands(commandBuffer);
    }

    vk::Buffer VulkanBuffer::GetBuffer()
    {
        return m_Buffer;
    }

    vk::DeviceMemory VulkanBuffer::GetDeviceMemory()
    {
        return m_BufferMemory;
    }

    void VulkanBuffer::BindForDraw()
    {
        throw new std::exception("VulkanBuffer::BindForDraw() not yet implemented!");
    }

    void VulkanBuffer::Unbind()
    {
        throw new std::exception("VulkanBuffer::Unbind() not yet implemented!");
    }

    void* VulkanBuffer::MapRead(uint32_t offset, uint32_t size)
    {
        throw new std::exception("VulkanBuffer::MapRead() not yet implemented!");
    }

    void VulkanBuffer::Unmap()
    {
        throw new std::exception("VulkanBuffer::Unmap() not yet implemented!");
    }

    void VulkanBuffer::Read(void* data, uint32_t offset, uint32_t size)
    {
        throw new std::exception("VulkanBuffer::Read() not yet implemented!");
    }
}
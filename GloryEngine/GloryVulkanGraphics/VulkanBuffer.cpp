#include "VulkanBuffer.h"
#include "VulkanGraphicsModule.h"
#include "Device.h"
#include "VulkanStructsConverter.h"
#include <Game.h>
#include <Engine.h>

namespace Glory
{
    VulkanBuffer::VulkanBuffer(uint32_t bufferSize, BufferBindingTarget usageFlag, MemoryUsage memoryFlags, uint32_t bindIndex) :
        Buffer(bufferSize, usageFlag, memoryFlags, bindIndex)
	{
	}

    VulkanBuffer::~VulkanBuffer()
	{
        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = pDeviceManager->GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        deviceData.LogicalDevice.destroyBuffer(m_Buffer);
        deviceData.LogicalDevice.freeMemory(m_BufferMemory);
	}

    void VulkanBuffer::CreateBuffer()
	{
        m_BufferCreateInfo = vk::BufferCreateInfo();
        m_BufferCreateInfo.size = (vk::DeviceSize)m_BufferSize;
        m_BufferCreateInfo.usage = VKConverter::TO_BUFFERUSAGE.at(m_UsageFlag); //vk::BufferUsageFlagBits::eVertexBuffer;
        m_BufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = pDeviceManager->GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        vk::Result result = deviceData.LogicalDevice.createBuffer(&m_BufferCreateInfo, nullptr, &m_Buffer);
        if (result != vk::Result::eSuccess)
            throw std::runtime_error("failed to create vertex buffer!");

        vk::MemoryRequirements memRequirements;
        deviceData.LogicalDevice.getBufferMemoryRequirements(m_Buffer, &memRequirements);

        uint32_t typeFilter = memRequirements.memoryTypeBits;
        vk::MemoryPropertyFlags properties = (vk::MemoryPropertyFlagBits)m_MemoryFlags; //vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        uint32_t memoryIndex = pDevice->GetSupportedMemoryIndex(typeFilter, properties);

        // Allocate device memory
        vk::MemoryAllocateInfo allocateInfo = vk::MemoryAllocateInfo();
        allocateInfo.allocationSize = memRequirements.size;
        allocateInfo.memoryTypeIndex = memoryIndex;

        result = deviceData.LogicalDevice.allocateMemory(&allocateInfo, nullptr, &m_BufferMemory);
        if (result != vk::Result::eSuccess)
            throw std::runtime_error("Failed to allocate vertex buffer memory!");

        deviceData.LogicalDevice.bindBufferMemory(m_Buffer, m_BufferMemory, 0);
	}

    void VulkanBuffer::Assign(const void* data)
    {
        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = pDeviceManager->GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        void* dstData;
        vk::Result result = deviceData.LogicalDevice.mapMemory(m_BufferMemory, (vk::DeviceSize)0, (vk::DeviceSize)m_BufferCreateInfo.size, (vk::MemoryMapFlags)0, &dstData);
        if (result != vk::Result::eSuccess)
            throw std::runtime_error("Failed to map memory!");
        memcpy(dstData, data, (size_t)m_BufferCreateInfo.size);
        deviceData.LogicalDevice.unmapMemory(m_BufferMemory);
    }

    void VulkanBuffer::Assign(const void* data, uint32_t offset, uint32_t size)
    {
        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = pDeviceManager->GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        void* dstData;
        vk::Result result = deviceData.LogicalDevice.mapMemory(m_BufferMemory, (vk::DeviceSize)offset, (vk::DeviceSize)size, (vk::MemoryMapFlags)0, &dstData);
        if (result != vk::Result::eSuccess)
            throw std::runtime_error("Failed to map memory!");
        memcpy(dstData, data, (size_t)m_BufferCreateInfo.size);
        deviceData.LogicalDevice.unmapMemory(m_BufferMemory);
    }

    void VulkanBuffer::CopyFrom(Buffer* source, uint32_t size)
    {
        // Memory transfer operations are executed using command buffers,
        // just like drawing commands. Therefore we must first allocate a temporary command buffer.
        // You may wish to create a separate command pool for these kinds of short-lived buffers,
        // because the implementation may be able to apply memory allocation optimizations.
        // You should use the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag
        // during command pool generation in that case.

        vk::CommandBuffer commandBuffer = VulkanGraphicsModule::BeginSingleTimeCommands();

        vk::BufferCopy copyRegion = vk::BufferCopy();
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        commandBuffer.copyBuffer(((VulkanBuffer*)source)->m_Buffer, m_Buffer, 1, &copyRegion);

        VulkanGraphicsModule::EndSingleTimeCommands(commandBuffer);
    }

    vk::Buffer VulkanBuffer::GetBuffer()
    {
        return m_Buffer;
    }

    vk::DeviceMemory VulkanBuffer::GetDeviceMemory()
    {
        return m_BufferMemory;
    }

    void VulkanBuffer::Bind()
    {
        assert(false);
    }

    void VulkanBuffer::Unbind()
    {
        assert(false);
    }
}
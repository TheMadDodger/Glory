#pragma once
#include <Buffer.h>
#include <vulkan/vulkan.hpp>

namespace Glory
{
	class VulkanBuffer : public Buffer
	{
	public:
		VulkanBuffer(uint32_t bufferSize, uint32_t usageFlag, uint32_t memoryFlags);
		virtual ~VulkanBuffer();

		virtual void CreateBuffer() override;
		virtual void Assign(const void* vertices) override;
		virtual void Assign(const void* vertices, uint32_t offset, uint32_t size) override;
		virtual void CopyFrom(Buffer* source, uint32_t size) override;

		vk::Buffer GetBuffer();
		vk::DeviceMemory GetDeviceMemory();

	private:
		vk::Buffer m_Buffer;
		vk::DeviceMemory m_BufferMemory;
		vk::BufferCreateInfo m_BufferCreateInfo;
	};
}

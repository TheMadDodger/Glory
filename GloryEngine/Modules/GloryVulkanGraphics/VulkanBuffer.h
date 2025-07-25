#pragma once
#include <Buffer.h>
#include <vulkan/vulkan.hpp>

namespace Glory
{
	class VulkanBuffer : public Buffer
	{
	public:
		VulkanBuffer(uint32_t bufferSize, BufferBindingTarget usageFlag, MemoryUsage memoryFlags, uint32_t bindIndex = 0);
		virtual ~VulkanBuffer();

		virtual void CreateBuffer() override;
		virtual void Assign(const void* vertices) override;
		virtual void Assign(const void* data, uint32_t size) override;
		virtual void Assign(const void* vertices, uint32_t offset, uint32_t size) override;
		virtual void CopyFrom(Buffer* source, uint32_t size) override;

		virtual void BindForDraw() override;
		virtual void Unbind() override;

		virtual void* MapRead(uint32_t offset, uint32_t size) override;
		virtual void Unmap() override;
		virtual void Read(void* data, uint32_t offset, uint32_t size) override;

		vk::Buffer GetBuffer();
		vk::DeviceMemory GetDeviceMemory();

	private:
		vk::Buffer m_Buffer;
		vk::DeviceMemory m_BufferMemory;
		vk::BufferCreateInfo m_BufferCreateInfo;
	};
}

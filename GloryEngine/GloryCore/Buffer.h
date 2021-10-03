#pragma once
#include <stdint.h>
#include "GPUResource.h"

namespace Glory
{
	class Buffer : public GPUResource
	{
	public:
		Buffer(uint32_t bufferSize, uint32_t usageFlag, uint32_t memoryFlags);
		virtual ~Buffer();

		virtual void Assign(const void* data) = 0;
		virtual void Assign(const void* data, uint32_t offset, uint32_t size) = 0;
		virtual void CopyFrom(Buffer* source, uint32_t size) = 0;
		static void Copy(Buffer* source, Buffer* dest, uint32_t size);

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

	protected:
		virtual void CreateBuffer() = 0;

	protected:
		uint32_t m_BufferSize;
		uint32_t m_UsageFlag;
		uint32_t m_MemoryFlags;

	private:
		friend class VulkanGraphicsModule;
	};
}
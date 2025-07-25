#pragma once
#include <stdint.h>
#include "GraphicsEnums.h"
#include "GPUResource.h"

namespace Glory
{
	class Buffer : public GPUResource
	{
	public:
		Buffer(size_t bufferSize, BufferBindingTarget usageFlag, MemoryUsage memoryFlags, size_t bindIndex);
		virtual ~Buffer();

		virtual void Assign(const void* data) = 0;
		virtual void Assign(const void* data, uint32_t size) = 0;
		virtual void Assign(const void* data, uint32_t offset, uint32_t size) = 0;
		virtual void CopyFrom(Buffer* source, uint32_t size) = 0;
		static void Copy(Buffer* source, Buffer* dest, uint32_t size);

		virtual void BindForDraw() = 0;
		virtual void Unbind() = 0;

		virtual void* MapRead(uint32_t offset, uint32_t size) = 0;
		virtual void Unmap() = 0;
		virtual void Read(void* data, uint32_t offset, uint32_t size) = 0;

		size_t Size() const;

	protected:
		virtual void CreateBuffer() = 0;

	protected:
		size_t m_BufferSize;
		BufferBindingTarget m_UsageFlag;
		MemoryUsage m_MemoryFlags;
		size_t m_BindIndex;

	private:
		friend class GPUResourceManager;
	};
}
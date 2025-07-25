#include "Buffer.h"

namespace Glory
{
	Buffer::Buffer(size_t bufferSize, BufferBindingTarget usageFlag, MemoryUsage memoryFlags, size_t bindIndex) :
		m_BufferSize(bufferSize), m_UsageFlag(usageFlag), m_MemoryFlags(memoryFlags), m_BindIndex(bindIndex)
	{
	}

	Buffer::~Buffer()
	{
	}

	void Buffer::Copy(Buffer* source, Buffer* dest, uint32_t size)
	{
		dest->CopyFrom(source, size);
	}

	size_t Buffer::Size() const
	{
		return m_BufferSize;
	}
}
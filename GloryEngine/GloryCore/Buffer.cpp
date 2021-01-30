#include "Buffer.h"

namespace Glory
{
	Buffer::Buffer(uint32_t bufferSize, uint32_t usageFlag, uint32_t memoryFlags) :
		m_BufferSize(bufferSize), m_UsageFlag(usageFlag), m_MemoryFlags(memoryFlags)
	{
	}

	Buffer::~Buffer()
	{
	}

	void Buffer::Copy(Buffer* source, Buffer* dest, uint32_t size)
	{
		dest->CopyFrom(source, size);
	}
}
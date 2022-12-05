#pragma once
#include <Buffer.h>
#include <GL/glew.h>

namespace Glory
{
	class GLBuffer : public Buffer
	{
	public:
		GLBuffer(uint32_t bufferSize, BufferBindingTarget usageFlag, MemoryUsage memoryFlags, size_t bindIndex);
		virtual ~GLBuffer();

		virtual void CreateBuffer() override;
		virtual void Assign(const void* data) override;
		virtual void Assign(const void* data, uint32_t offset, uint32_t size) override;
		virtual void CopyFrom(Buffer* source, uint32_t size) override;

		virtual void Bind() override;
		virtual void Unbind() override;

	private:
		GLuint m_BufferID;
	};
}

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
		virtual void Assign(const void* data, uint32_t size) override;
		virtual void Assign(const void* data, uint32_t offset, uint32_t size) override;
		virtual void CopyFrom(Buffer* source, uint32_t size) override;

		virtual void BindForDraw() override;
		virtual void Unbind() override;

		virtual void* MapRead(uint32_t offset, uint32_t size) override;
		virtual void Unmap() override;
		virtual void Read(void* data, uint32_t offset, uint32_t size) override;

	private:
		GLuint m_BufferID;
	};
}

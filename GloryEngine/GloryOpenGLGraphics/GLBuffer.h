#pragma once
#include <Buffer.h>
#include <GL/glew.h>

namespace Glory
{
	class GLBuffer : public Buffer
	{
	public:
		GLBuffer(uint32_t bufferSize, uint32_t usageFlag, uint32_t memoryFlags);
		virtual ~GLBuffer();

		virtual void CreateBuffer() override;
		virtual void Assign(const void* vertices) override;
		virtual void Assign(const void* vertices, uint32_t offset, uint32_t size) override;
		virtual void CopyFrom(Buffer* source, uint32_t size) override;

		virtual void Bind() override;
		virtual void Unbind() override;

	private:
		GLuint m_BufferID;
	};
}

#include "GLBuffer.h"
#include "OpenGLGraphicsModule.h"	

namespace Glory
{
	GLBuffer::GLBuffer(uint32_t bufferSize, uint32_t usageFlag, uint32_t memoryFlags) :
		Buffer(bufferSize, usageFlag, memoryFlags)
	{
	}

	GLBuffer::~GLBuffer()
	{
		if (m_BufferID == 0) return;
		glDeleteBuffers(1, &m_BufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void GLBuffer::CreateBuffer()
	{
		glGenBuffers(1, &m_BufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void GLBuffer::Assign(const void* vertices)
	{
		//GL_ARRAY_BUFFER
		//GL_ELEMENT_ARRAY_BUFFER

		//GL_STATIC_DRAW

		glBindBuffer(m_UsageFlag, m_BufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferData(m_UsageFlag, m_BufferSize, vertices, m_MemoryFlags);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(m_UsageFlag, NULL);
	}

	void GLBuffer::Assign(const void* vertices, uint32_t offset, uint32_t size)
	{
		glBindBuffer(m_UsageFlag, m_BufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferSubData(m_UsageFlag, offset, m_BufferSize, vertices);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(m_UsageFlag, NULL);
	}

	void GLBuffer::CopyFrom(Buffer* source, uint32_t size)
	{
		
	}

	void GLBuffer::Bind()
	{
		glBindBuffer(m_UsageFlag, m_BufferID);
	}

	void GLBuffer::Unbind()
	{
		glBindBuffer(m_UsageFlag, NULL);
	}
}
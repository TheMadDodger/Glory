#include "GLBuffer.h"
#include "OpenGLGraphicsModule.h"	

namespace Glory
{
	GLBuffer::GLBuffer(uint32_t bufferSize, uint32_t usageFlag, uint32_t memoryFlags, size_t bindIndex) :
		Buffer(bufferSize, usageFlag, memoryFlags, bindIndex)
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

	void GLBuffer::Assign(const void* data)
	{
		//GL_ARRAY_BUFFER
		//GL_ELEMENT_ARRAY_BUFFER

		//GL_STATIC_DRAW

		glBindBuffer(m_UsageFlag, m_BufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferData(m_UsageFlag, m_BufferSize, data, m_MemoryFlags);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(m_UsageFlag, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void GLBuffer::Assign(const void* data, uint32_t offset, uint32_t size)
	{
		glBindBuffer(m_UsageFlag, m_BufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferSubData(m_UsageFlag, offset, m_BufferSize, data);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(m_UsageFlag, NULL);
	}

	void GLBuffer::CopyFrom(Buffer* source, uint32_t size)
	{
		
	}

	void GLBuffer::Bind()
	{
		glBindBuffer(m_UsageFlag, m_BufferID);

		if (m_BindIndex)
		{
			glBindBufferBase(m_UsageFlag, (GLuint)m_BindIndex, m_BufferID);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}
	}

	void GLBuffer::Unbind()
	{
		glBindBuffer(m_UsageFlag, NULL);
	}
}
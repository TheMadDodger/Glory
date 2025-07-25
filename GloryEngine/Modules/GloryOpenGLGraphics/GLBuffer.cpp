#include "GLBuffer.h"
#include "OpenGLGraphicsModule.h"	
#include "GLConverter.h"
#include "GloryOGL.h"

namespace Glory
{
	GLBuffer::GLBuffer(uint32_t bufferSize, BufferBindingTarget usageFlag, MemoryUsage memoryFlags, size_t bindIndex) :
		Buffer(bufferSize, usageFlag, memoryFlags, bindIndex), m_BufferID(0)
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

		const GLuint target = GLConverter::TO_GLBUFFERTARGET.at(m_UsageFlag);
		const GLuint usage = GLConverter::TO_GLBUFFERUSAGE.at(m_MemoryFlags);

		glBindBuffer(target, m_BufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferData(target, m_BufferSize, data, usage);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(target, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void GLBuffer::Assign(const void* data, uint32_t size)
	{
		const GLuint target = GLConverter::TO_GLBUFFERTARGET.at(m_UsageFlag);
		const GLuint usage = GLConverter::TO_GLBUFFERUSAGE.at(m_MemoryFlags);

		glBindBuffer(target, m_BufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		if (size > m_BufferSize)
		{
			m_BufferSize = size;
			glBufferData(target, m_BufferSize, data, usage);
		}
		else
			glBufferSubData(target, 0, size, data);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(target, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void GLBuffer::Assign(const void* data, uint32_t offset, uint32_t size)
	{
		GLuint target = GLConverter::TO_GLBUFFERTARGET.at(m_UsageFlag);

		glBindBuffer(target, m_BufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		if (offset + size > m_BufferSize)
		{
			/* @todo: Log an error or warning */
		}
		glBufferSubData(target, offset, size, data);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(target, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void GLBuffer::CopyFrom(Buffer* source, uint32_t size)
	{
		
	}

	void GLBuffer::BindForDraw()
	{
		GLuint target = GLConverter::TO_GLBUFFERTARGET.at(m_UsageFlag);
		glBindBuffer(target, m_BufferID);

		if (m_BindIndex)
		{
			glBindBufferBase(target, (GLuint)m_BindIndex, m_BufferID);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}
	}

	void GLBuffer::Unbind()
	{
		GLuint target = GLConverter::TO_GLBUFFERTARGET.at(m_UsageFlag);
		glBindBuffer(target, NULL);
	}

	void* GLBuffer::MapRead(uint32_t offset, uint32_t size)
	{
		const GLuint target = GLConverter::TO_GLBUFFERTARGET.at(m_UsageFlag);
		glBindBuffer(target, m_BufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		void* pMemory = glMapBufferRange(target, offset, size, GL_MAP_READ_BIT);
		OpenGLGraphicsModule::LogGLError(glGetError());

		return pMemory;
	}

	void GLBuffer::Unmap()
	{
		const GLuint target = GLConverter::TO_GLBUFFERTARGET.at(m_UsageFlag);
		glUnmapBuffer(target);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindBuffer(target, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void GLBuffer::Read(void* data, uint32_t offset, uint32_t size)
	{
		const GLuint target = GLConverter::TO_GLBUFFERTARGET.at(m_UsageFlag);
		glBindBuffer(target, m_BufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glGetBufferSubData(target, offset, size, data);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindBuffer(target, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}
}
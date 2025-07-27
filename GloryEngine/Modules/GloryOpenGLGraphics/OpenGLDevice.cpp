#include "OpenGLDevice.h"

#include "OpenGLGraphicsModule.h"
#include "GLShader.h"
#include "OGLMaterial.h"
#include "OGLPipeline.h"
#include "OGLRenderTexture.h"

#include <Engine.h>
#include <Debug.h>
#include <CubemapData.h>
#include <PipelineData.h>

namespace Glory
{
	OpenGLDevice::OpenGLDevice(OpenGLGraphicsModule* pModule): GraphicsDevice(pModule)
	{
	}

	OpenGLDevice::~OpenGLDevice()
	{
		m_Buffers.Clear();
		m_Meshes.Clear();
	}

	OpenGLGraphicsModule* OpenGLDevice::GraphicsModule()
	{
		return static_cast<OpenGLGraphicsModule*>(m_pModule);
	}

#pragma region Resource Management

	BufferHandle OpenGLDevice::CreateBuffer(size_t bufferSize, BufferType type)
	{
		BufferHandle handle;
		GL_Buffer& buffer = m_Buffers.Emplace(handle, GL_Buffer());
		glGenBuffers(1, &buffer.m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		switch (type)
		{
		case Glory::BT_TransferRead:
			buffer.m_Target = GL_COPY_READ_BUFFER;
			buffer.m_Usage = GL_DYNAMIC_COPY;
			break;
		case Glory::BT_TransferWrite:
			buffer.m_Target = GL_COPY_WRITE_BUFFER;
			buffer.m_Usage = GL_DYNAMIC_COPY;
			break;
		case Glory::BT_Vertex:
			buffer.m_Target = GL_ARRAY_BUFFER;
			buffer.m_Usage = GL_STATIC_DRAW;
			break;
		case Glory::BT_Index:
			buffer.m_Target = GL_ELEMENT_ARRAY_BUFFER;
			buffer.m_Usage = GL_STATIC_DRAW;
			break;
		default:
			break;
		}

		glBindBuffer(buffer.m_Target, buffer.m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferData(buffer.m_Target, buffer.m_Size, NULL, buffer.m_Usage);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(buffer.m_Target, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		std::stringstream str;
		str << "OpenGLDevice: Buffer " << handle << " created with size " << bufferSize;
		Debug().LogInfo(str.str());

		return handle;
	}

	void OpenGLDevice::AssignBuffer(BufferHandle handle, const void* data)
	{
		GL_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("OpenGLDevice::AssignBuffer: Invalid buffer handle");
			return;
		}

		glBindBuffer(buffer->m_Target, buffer->m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferData(buffer->m_Target, buffer->m_Size, data, buffer->m_Usage);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(buffer->m_Target, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLDevice::AssignBuffer(BufferHandle handle, const void* data, uint32_t size)
	{
		GL_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("OpenGLDevice::AssignBuffer: Invalid buffer handle");
			return;
		}

		glBindBuffer(buffer->m_Target, buffer->m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		if (size > buffer->m_Size)
		{
			buffer->m_Size = size;
			glBufferData(buffer->m_Target, buffer->m_Size, data, buffer->m_Usage);
		}
		else
			glBufferSubData(buffer->m_Target, 0, size, data);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(buffer->m_Target, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLDevice::AssignBuffer(BufferHandle handle, const void* data, uint32_t offset, uint32_t size)
	{
		GL_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("OpenGLDevice::AssignBuffer: Invalid buffer handle");
			return;
		}

		if (offset + size > buffer->m_Size)
		{
			Debug().LogError("OpenGLDevice::AssignBuffer: Attempting to write beyong buffer size");
			return;
		}

		glBindBuffer(buffer->m_Target, buffer->m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferSubData(buffer->m_Target, offset, size, data);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(buffer->m_Target, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	MeshHandle OpenGLDevice::CreateMesh(std::vector<BufferHandle>&& buffers, uint32_t vertexCount,
		uint32_t indexCount, uint32_t stride, PrimitiveType primitiveType,
		const std::vector<AttributeType>& attributeTypes)
	{
		MeshHandle handle;
		GL_Mesh& mesh = m_Meshes.Emplace(handle, GL_Mesh());
		mesh.m_Buffers = std::move(buffers);

		glGenVertexArrays(1, &mesh.m_GLVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindVertexArray(mesh.m_GLVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		std::vector<GL_Buffer*> glBuffers;

		for (auto& bufferHandle : mesh.m_Buffers)
		{
			GL_Buffer* buffer = m_Buffers.Find(bufferHandle);
			if (!buffer)
			{
				Debug().LogError("OpenGLDevice::CreateMesh: Invalid buffer handle");
				glBindVertexArray(NULL);
				OpenGLGraphicsModule::LogGLError(glGetError());
				glDeleteVertexArrays(1, &mesh.m_GLVertexArrayID);
				OpenGLGraphicsModule::LogGLError(glGetError());
				m_Meshes.Erase(handle);
				return NULL;
			}
			glBindBuffer(buffer->m_Target, buffer->m_GLBufferID);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glBuffers.emplace_back(buffer);
		}

		size_t offset = 0;
		for (uint32_t i = 0; i < attributeTypes.size(); i++)
		{
			switch (attributeTypes[i])
			{
			case Glory::AttributeType::Float:
				glVertexAttribPointer(i, 1, GL_FLOAT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 1*sizeof(GLfloat);
				break;
			case Glory::AttributeType::Float2:
				glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 2*sizeof(GLfloat);
				break;
			case Glory::AttributeType::Float3:
				glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 3*sizeof(GLfloat);
				break;
			case Glory::AttributeType::Float4:
				glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 4*sizeof(GLfloat);
				break;
			case Glory::AttributeType::UINT:
				glVertexAttribPointer(i, 1, GL_UNSIGNED_INT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 1*sizeof(GLuint);
				break;
			case Glory::AttributeType::UINT2:
				glVertexAttribPointer(i, 2, GL_UNSIGNED_INT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 2*sizeof(GLuint);
				break;
			case Glory::AttributeType::UINT3:
				glVertexAttribPointer(i, 3, GL_UNSIGNED_INT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 3*sizeof(GLuint);
				break;
			case Glory::AttributeType::UINT4:
				glVertexAttribPointer(i, 4, GL_UNSIGNED_INT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 4*sizeof(GLuint);
				break;
			case Glory::AttributeType::SINT:
				glVertexAttribPointer(i, 1, GL_INT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 1*sizeof(GLint);
				break;
			case Glory::AttributeType::SINT2:
				glVertexAttribPointer(i, 2, GL_INT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 2*sizeof(GLint);
				break;
			case Glory::AttributeType::SINT3:
				glVertexAttribPointer(i, 3, GL_INT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 3*sizeof(GLint);
				break;
			case Glory::AttributeType::SINT4:
				glVertexAttribPointer(i, 4, GL_INT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 4*sizeof(GLint);
				break;
			default:
				break;
			}

			glEnableVertexAttribArray(i);
		}

		glBindVertexArray(NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		for (auto buffer : glBuffers)
		{
			glBindBuffer(buffer->m_Target, NULL);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		std::stringstream str;
		str << "OpenGLDevice: Mesh " << handle << " created";
		Debug().LogInfo(str.str());

		return handle;
	}

	void OpenGLDevice::FreeBuffer(BufferHandle& handle)
	{
		GL_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("OpenGLDevice::FreeBuffer: Invalid buffer handle");
			return;
		}

		glDeleteBuffers(1, &buffer->m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		m_Buffers.Erase(handle);

		std::stringstream str;
		str << "OpenGLDevice: Buffer " << handle << " was freed from device memory";
		Debug().LogInfo(str.str());

		handle = 0;
	}

	void OpenGLDevice::FreeMesh(MeshHandle& handle)
	{
		GL_Mesh* mesh = m_Meshes.Find(handle);
		if (!mesh)
		{
			Debug().LogError("OpenGLDevice::FreeMesh: Invalid mesh handle");
			return;
		}

		glDeleteVertexArrays(1, &mesh->m_GLVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		for (auto& buffer : mesh->m_Buffers)
			FreeBuffer(buffer);

		m_Meshes.Erase(handle);

		std::stringstream str;
		str << "OpenGLDevice: Mesh " << handle << " was freed from device memory";
		Debug().LogInfo(str.str());

		handle = 0;
	}

#pragma endregion
}

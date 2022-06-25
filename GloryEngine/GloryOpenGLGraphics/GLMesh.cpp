#include "GLMesh.h"
#include "OpenGLGraphicsModule.h"
#include <Debug.h>
#include "GloryOGL.h"

namespace Glory
{
	GLMesh::GLMesh(size_t vertexCount, size_t indexCount, InputRate inputRate, size_t binding, size_t stride, const std::vector<AttributeType>& attributeTypes) :
		Mesh(vertexCount, indexCount, inputRate, binding, stride, attributeTypes), m_VertexArrayID(0)
	{
	}

	GLMesh::~GLMesh()
	{
		if (m_VertexArrayID == 0) return;
		glDeleteVertexArrays(1, &m_VertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		m_VertexArrayID = 0;
	}

	void GLMesh::CreateBindingAndAttributeData()
	{
		glGenVertexArrays(1, &m_VertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindVertexArray(m_VertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		m_pVertexBuffer->Bind();
		m_pIndexBuffer->Bind();

		SetupAttributes();

		glBindVertexArray(NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		m_pVertexBuffer->Unbind();
		m_pIndexBuffer->Unbind();
	}

	void GLMesh::Bind()
	{
		glBindVertexArray(m_VertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void GLMesh::SetupAttributes()
	{
		size_t offset = 0;
		for (size_t i = 0; i < m_AttributeTypes.size(); i++)
		{
			glEnableVertexAttribArray(i);
			OpenGLGraphicsModule::LogGLError(glGetError());

			AttributeType attribute = m_AttributeTypes[i];

			switch (attribute)
			{
			case Glory::AttributeType::Float:
				glVertexAttribPointer(i, 1, GL_FLOAT, GL_FALSE, m_Stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 1 * sizeof(GLfloat);
				break;
			case Glory::AttributeType::Float2:
				glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, m_Stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 2 * sizeof(GLfloat);
				break;
			case Glory::AttributeType::Float3:
				glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, m_Stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 3 * sizeof(GLfloat);
				break;
			case Glory::AttributeType::Float4:
				glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, m_Stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 4 * sizeof(GLfloat);
				break;
			case Glory::AttributeType::UINT:
				glVertexAttribPointer(i, 1, GL_UNSIGNED_INT, GL_FALSE, m_Stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 1 * sizeof(GLuint);
				break;
			case Glory::AttributeType::UINT2:
				glVertexAttribPointer(i, 2, GL_UNSIGNED_INT, GL_FALSE, m_Stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 2 * sizeof(GLuint);
				break;
			case Glory::AttributeType::UINT3:
				glVertexAttribPointer(i, 3, GL_UNSIGNED_INT, GL_FALSE, m_Stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 3 * sizeof(GLuint);
				break;
			case Glory::AttributeType::UINT4:
				glVertexAttribPointer(i, 4, GL_UNSIGNED_INT, GL_FALSE, m_Stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 4 * sizeof(GLuint);
				break;
			case Glory::AttributeType::SINT:
				glVertexAttribPointer(i, 1, GL_INT, GL_FALSE, m_Stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 1 * sizeof(GLint);
				break;
			case Glory::AttributeType::SINT2:
				glVertexAttribPointer(i, 2, GL_INT, GL_FALSE, m_Stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 2 * sizeof(GLint);
				break;
			case Glory::AttributeType::SINT3:
				glVertexAttribPointer(i, 3, GL_INT, GL_FALSE, m_Stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 3 * sizeof(GLint);
				break;
			case Glory::AttributeType::SINT4:
				glVertexAttribPointer(i, 4, GL_INT, GL_FALSE, m_Stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 4 * sizeof(GLint);
				break;
			default:
				break;
			}
		}
	}
}
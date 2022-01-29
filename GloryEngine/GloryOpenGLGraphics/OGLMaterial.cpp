#include "OGLMaterial.h"
#include "OpenGLGraphicsModule.h"
#include "Debug.h"
#include "GLShader.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Glory
{
	OGLMaterial::OGLMaterial(MaterialData* pMaterialData)
		: Material(pMaterialData), m_ProgramID(NULL), m_UBOID(NULL)
	{
	}

	OGLMaterial::~OGLMaterial()
	{
		if (m_UniformBufferObjects.size() > 0)
		{
			glDeleteBuffers(m_UniformBufferObjects.size(), &m_UniformBufferObjects[0]);
			m_UniformBufferObjects.clear();
		}
	}

	void OGLMaterial::Use()
	{
		m_TextureCounter = 0;
		glUseProgram(m_ProgramID);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLMaterial::Initialize()
	{
		int success;
		char infoLog[512];

		m_ProgramID = glCreateProgram();
		OpenGLGraphicsModule::LogGLError(glGetError());

		for (size_t i = 0; i < m_pShaders.size(); i++)
		{
			GLShader* pShader = (GLShader*)m_pShaders[i];
			glAttachShader(m_ProgramID, pShader->m_ShaderID);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		glLinkProgram(m_ProgramID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &success);
		OpenGLGraphicsModule::LogGLError(glGetError());
		if (!success)
		{
			glGetProgramInfoLog(m_ProgramID, 512, NULL, infoLog);
			OpenGLGraphicsModule::LogGLError(glGetError());
			Debug::LogError(infoLog);
		}
	}

	void OGLMaterial::SetPropertiesExtra()
	{
		if (m_UBOID == NULL) m_UBOID = CreateUniformBuffer("UniformBufferObject", sizeof(UniformBufferObjectTest), 0);
		SetUniformBuffer(m_UBOID, (void*)&m_UBO, sizeof(UniformBufferObjectTest));
	}

	void OGLMaterial::SetFloat(const std::string& name, float value) const
	{
		GLint ID = glGetUniformLocation(m_ProgramID, name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniform1f(ID, value);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLMaterial::SetInt(const std::string& name, int value) const
	{
		GLint ID = glGetUniformLocation(m_ProgramID, name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniform1i(ID, value);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLMaterial::SetUInt(const std::string& name, uint32_t value) const
	{
		GLint ID = glGetUniformLocation(m_ProgramID, name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniform1ui(ID, value);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLMaterial::SetIntArray(const std::string& name, int size, int* value) const
	{
		GLint ID = glGetUniformLocation(m_ProgramID, name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniform1iv(ID, size, value);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLMaterial::SetBool(const std::string& name, bool value) const
	{
		GLint ID = glGetUniformLocation(m_ProgramID, name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniform1i(ID, (int)value);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLMaterial::SetVec2(const std::string& name, const glm::vec2& value) const
	{
		GLint ID = glGetUniformLocation(m_ProgramID, name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniform2f(ID, value.x, value.y);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLMaterial::SetVec3(const std::string& name, const glm::vec3& value) const
	{
		GLint ID = glGetUniformLocation(m_ProgramID, name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniform3f(ID, value.x, value.y, value.z);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLMaterial::SetUVec3(const std::string& name, const glm::uvec3& value) const
	{
		GLint ID = glGetUniformLocation(m_ProgramID, name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniform3ui(ID, value.x, value.y, value.z);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLMaterial::SetVec4(const std::string& name, const glm::vec4& value) const
	{
		GLint ID = glGetUniformLocation(m_ProgramID, name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniform4f(ID, value.x, value.y, value.z, value.w);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLMaterial::SetDouble(const std::string& name, double value) const
	{
		GLint ID = glGetUniformLocation(m_ProgramID, name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniform1d(ID, value);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLMaterial::SetMatrix3(const std::string& name, const glm::mat3& value) const
	{
		GLint ID = glGetUniformLocation(m_ProgramID, name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());

		const float* pSource = (const float*)glm::value_ptr(value);
		glUniformMatrix3fv(ID, 1, GL_FALSE, pSource);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLMaterial::SetMatrix4(const std::string& name, const glm::mat4& value) const
	{
		GLint ID = glGetUniformLocation(m_ProgramID, name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());

		const float* pSource = (const float*)glm::value_ptr(value);
		glUniformMatrix4fv(ID, 1, GL_FALSE, pSource);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLMaterial::SetTexture(const std::string& name, Texture* pTexture)
	{
		GLuint texLocation = glGetUniformLocation(m_ProgramID, name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniform1i(texLocation, m_TextureCounter);
		OpenGLGraphicsModule::LogGLError(glGetError());

		GLTexture* pGLTexture = (GLTexture*)pTexture;
		glActiveTexture(GL_TEXTURE0 + m_TextureCounter);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindTexture(GL_TEXTURE_2D, pGLTexture->GetID());
		OpenGLGraphicsModule::LogGLError(glGetError());

		glActiveTexture(GL_TEXTURE0);
		OpenGLGraphicsModule::LogGLError(glGetError());

		++m_TextureCounter;
	}

	void OGLMaterial::SetTexture(const std::string& name, GLuint id)
	{
		GLuint texLocation = glGetUniformLocation(m_ProgramID, name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniform1i(texLocation, m_TextureCounter);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glActiveTexture(GL_TEXTURE0 + m_TextureCounter);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindTexture(GL_TEXTURE_2D, id);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glActiveTexture(GL_TEXTURE0);
		OpenGLGraphicsModule::LogGLError(glGetError());

		++m_TextureCounter;
	}

	GLuint OGLMaterial::CreateUniformBuffer(const std::string& name, GLuint bufferSize, GLuint bindingIndex)
	{
		GLuint uniformBlockIndex = glGetUniformBlockIndex(m_ProgramID, name.data());
		OpenGLGraphicsModule::LogGLError(glGetError());
		if (uniformBlockIndex == GL_INVALID_INDEX)
		{
			Debug::LogError("OGLMaterial::CreateUniformBuffer > Uniform buffer block with name " + name + " not found!");
			return 0;
		}

		// Manually bind the uniform block
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniformBlockBinding(m_ProgramID, uniformBlockIndex, bindingIndex);
		OpenGLGraphicsModule::LogGLError(glGetError());

		// Uniform buffer object for lights
		GLuint bufferID;
		glGenBuffers(1, &bufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(GL_UNIFORM_BUFFER, bufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferData(GL_UNIFORM_BUFFER, bufferSize, NULL, GL_DYNAMIC_DRAW);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBufferBase(GL_UNIFORM_BUFFER, uniformBlockIndex, bufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		OpenGLGraphicsModule::LogGLError(glGetError());

		m_UniformBufferObjects.push_back(bufferID);
		return bufferID;
	}

	void OGLMaterial::SetUniformBuffer(GLuint bufferID, void* data, GLuint size)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, bufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	//void OGLMaterial::SetTexture(const std::string& name, GLTexture* pTexture)
	//{
	//	GLuint texLocation = glGetUniformLocation(m_ProgramID, name.c_str());
	//	OpenGLGraphicsModule::LogGLError(glGetError());
	//	glUniform1i(texLocation, m_TextureCounter);
	//	OpenGLGraphicsModule::LogGLError(glGetError());
	//
	//	glActiveTexture(GL_TEXTURE0 + m_TextureCounter);
	//	OpenGLGraphicsModule::LogGLError(glGetError());
	//	glBindTexture(pTexture->GetGLImageType(), pTexture->GetID());
	//	OpenGLGraphicsModule::LogGLError(glGetError());
	//
	//	glActiveTexture(GL_TEXTURE0);
	//	OpenGLGraphicsModule::LogGLError(glGetError());
	//
	//	++m_TextureCounter;
	//}
}

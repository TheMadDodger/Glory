#include "OGLMaterial.h"
#include "OpenGLGraphicsModule.h"
#include "Debug.h"
#include "GLShader.h"
#include "OGLPipeline.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Engine.h>
#include "GloryOGL.h"

namespace Glory
{
	OGLMaterial::OGLMaterial(MaterialData* pMaterialData)
		: Material(pMaterialData), m_UBOID(NULL), m_TextureCounter(0)
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
		if (!m_pPipeline) return;
		m_pPipeline->Use();
		m_TextureCounter = 0;
		m_TextureSetBits = 0;
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	Buffer* OGLMaterial::CreatePropertiesBuffer(uint32_t size)
	{
		return m_pOwner->GetEngine()->GetMainModule<GraphicsModule>()->GetResourceManager()
			->CreateBuffer(size, BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_DYNAMIC_DRAW, 1);
	}

	Buffer* OGLMaterial::CreateHasTexturesBuffer(uint32_t bindingIndex)
	{
		return m_pOwner->GetEngine()->GetMainModule<GraphicsModule>()->GetResourceManager()
			->CreateBuffer((uint32_t)sizeof(uint64_t), BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_DYNAMIC_DRAW, bindingIndex);
	}

	Buffer* OGLMaterial::CreateMVPBuffer()
	{
		return m_pOwner->GetEngine()->GetMainModule<GraphicsModule>()->GetResourceManager()
			->CreateBuffer((uint32_t)sizeof(ObjectData), BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_DYNAMIC_DRAW, 2);
	}

	void OGLMaterial::SetFloat(const std::string& name, float value) const
	{
		if (!Pipeline()) return;

		GLint ID = glGetUniformLocation(Pipeline()->ProgramID(), name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniform1f(ID, value);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLMaterial::SetFloat3(const std::string& name, const glm::vec3& value) const
	{
		if (!Pipeline()) return;

		GLint ID = glGetUniformLocation(Pipeline()->ProgramID(), name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniform3f(ID, value.x, value.y, value.z);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLMaterial::SetMatrix4(const std::string& name, const glm::mat4& value) const
	{
		if (!Pipeline()) return;

		GLint ID = glGetUniformLocation(Pipeline()->ProgramID(), name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());

		const float* pSource = (const float*)glm::value_ptr(value);
		glUniformMatrix4fv(ID, 1, GL_FALSE, pSource);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OGLMaterial::SetTexture(const std::string& name, Texture* pTexture)
	{
		if (!Pipeline()) return;

		GLuint texLocation = glGetUniformLocation(Pipeline()->ProgramID(), name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniform1i(texLocation, m_TextureCounter);
		OpenGLGraphicsModule::LogGLError(glGetError());

		GLTexture* pGLTexture = (GLTexture*)pTexture;
		glActiveTexture(GL_TEXTURE0 + m_TextureCounter);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindTexture(GL_TEXTURE_2D, pGLTexture ? pGLTexture->GetID() : 0);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glActiveTexture(GL_TEXTURE0);
		OpenGLGraphicsModule::LogGLError(glGetError());

		if (pTexture)
			m_TextureSetBits |= 1 << m_TextureCounter;
		++m_TextureCounter;
	}

	void OGLMaterial::SetCubemapTexture(const std::string& name, Texture* pTexture)
	{
		if (!Pipeline()) return;

		GLuint texLocation = glGetUniformLocation(Pipeline()->ProgramID(), name.c_str());
		OpenGLGraphicsModule::LogGLError(glGetError());
		glUniform1i(texLocation, m_TextureCounter);
		OpenGLGraphicsModule::LogGLError(glGetError());

		GLTexture* pGLTexture = (GLTexture*)pTexture;
		glActiveTexture(GL_TEXTURE0 + m_TextureCounter);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindTexture(GL_TEXTURE_CUBE_MAP, pGLTexture ? pGLTexture->GetID() : 0);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glActiveTexture(GL_TEXTURE0);
		OpenGLGraphicsModule::LogGLError(glGetError());

		if (pTexture)
			m_TextureSetBits |= 1 << m_TextureCounter;
		++m_TextureCounter;
	}

	void OGLMaterial::ResetTextureCounter()
	{
		m_TextureCounter = 0;
	}

	void OGLMaterial::SetTexture(const std::string& name, GLuint id)
	{
		if (!Pipeline()) return;

		GLuint texLocation = glGetUniformLocation(Pipeline()->ProgramID(), name.c_str());
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

	OGLPipeline* OGLMaterial::Pipeline() const
	{
		if (!m_pPipeline) return nullptr;
		return static_cast<OGLPipeline*>(m_pPipeline);
	}
}

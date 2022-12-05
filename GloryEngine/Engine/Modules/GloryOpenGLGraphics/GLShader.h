#pragma once
#include <GL/glew.h>
#include <Shader.h>

namespace Glory
{
	class GLShader : public Shader
	{
	public:
		GLShader(FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function);
		virtual ~GLShader();

	private:
		virtual void Initialize() override;

	private:
		friend class OGLMaterial;
		GLuint m_ShaderID;
		GLuint m_GLShaderType;
	};
}

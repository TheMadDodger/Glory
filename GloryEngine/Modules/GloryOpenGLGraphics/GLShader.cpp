#include "GLShader.h"
#include "OpenGLGraphicsModule.h"
#include "GLConverter.h"
#include "GloryOGL.h"

#include <Debug.h>
#include <Engine.h>

namespace Glory
{
	GLShader::GLShader(const FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function)
		: Shader(pShaderFileData, shaderType, function), m_ShaderID(NULL)
	{
	}

	GLShader::~GLShader()
	{
		glDeleteShader(m_ShaderID);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void GLShader::Initialize()
	{
		const char* shaderSource = m_CompiledShader.data();

		m_GLShaderType = GLConverter::GetShaderStageFlag(m_ShaderType);
		m_ShaderID = glCreateShader(m_GLShaderType);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glShaderSource(m_ShaderID, 1, &shaderSource, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glCompileShader(m_ShaderID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		int success;
		char infoLog[512];
		glGetShaderiv(m_ShaderID, GL_COMPILE_STATUS, &success);
		OpenGLGraphicsModule::LogGLError(glGetError());

		if (!success)
		{
			glGetShaderInfoLog(m_ShaderID, 512, NULL, infoLog);
			OpenGLGraphicsModule::LogGLError(glGetError());
			m_pOwner->GetEngine()->GetDebug().LogError(infoLog);
		}
	}
}

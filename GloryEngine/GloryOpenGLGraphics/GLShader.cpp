#include "GLShader.h"
#include "OpenGLGraphicsModule.h"
#include <Debug.h>
#include "GLConverter.h"

namespace Glory
{
	GLShader::GLShader(FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function)
		: m_pShaderFileData(pShaderFileData), m_ShaderType(shaderType), m_Function(function), m_ShaderID(NULL)
	{
	}

	GLShader::~GLShader()
	{
		glDeleteShader(m_ShaderID);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void GLShader::Initialize()
	{
		const char* shaderSource = m_pShaderFileData->Data();

		m_ShaderID = glCreateShader(GLConverter::GetShaderStageFlag(m_ShaderType));
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
			Debug::LogError(infoLog);
		}
	}
}

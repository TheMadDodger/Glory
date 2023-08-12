#include "Shader.h"

namespace Glory
{
	Shader::Shader(FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function)
		: m_pShaderFileData(pShaderFileData), m_ShaderType(shaderType), m_Function(function)
	{
	}

	Shader::~Shader()
	{
	}
}

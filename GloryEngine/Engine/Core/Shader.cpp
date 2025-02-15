#include "Shader.h"

namespace Glory
{
	Shader::Shader(const FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function)
		: m_CompiledShader(pShaderFileData->Begin(), pShaderFileData->End()), m_ShaderType(shaderType), m_Function(function)
	{
	}

	Shader::~Shader()
	{
	}
}

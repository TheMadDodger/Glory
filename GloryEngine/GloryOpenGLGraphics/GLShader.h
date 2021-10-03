#pragma once
#include <GL/glew.h>
#include <FileData.h>
#include <GraphicsEnums.h>
#include <string>

namespace Glory
{
	class GLShader
	{
	public:
		GLShader(FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function);
		virtual ~GLShader();

	private:
		void Initialize();

	private:
		friend class OpenGLGraphicsModule;
		FileData* m_pShaderFileData;
		const ShaderType m_ShaderType;
		const std::string m_Function;
		GLuint m_ShaderID;
	};
}

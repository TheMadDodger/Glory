#pragma once
#include "GPUResource.h"
#include "FileData.h"
#include "GraphicsEnums.h"
#include <string>

/*
* Need to merge the shader and shader data class into one class
* Which means the loader will have to ask the graphics module to make a shader object,
* and after filling in the data call an initialize function
*/

namespace Glory
{
	class Shader : public GPUResource
	{
	public:
		Shader(const FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function);
		virtual ~Shader();

	protected:
		virtual void Initialize() = 0;

	protected:
		std::vector<char> m_CompiledShader;
		const ShaderType m_ShaderType;
		const std::string m_Function;

	private:
		friend class GPUResourceManager;
	};
}

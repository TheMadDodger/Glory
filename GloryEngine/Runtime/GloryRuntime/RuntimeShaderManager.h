#pragma once
#include <ShaderManager.h>

namespace Glory
{
	class RuntimeShaderManager : public ShaderManager
	{
	public:
		RuntimeShaderManager(Engine* pEngine);
		virtual ~RuntimeShaderManager() = default;

	private:
		virtual ShaderType GetShaderType(UUID shaderID) const override;
		virtual FileData* GetCompiledShaderFile(UUID shaderID) const override;
	};
}

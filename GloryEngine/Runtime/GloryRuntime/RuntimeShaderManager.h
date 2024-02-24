#pragma once
#include <ShaderManager.h>

#include <map>

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

		virtual void AddShader(ShaderSourceData* pShader) override;

	private:
		std::map<UUID, ShaderSourceData*> m_pShaders;
	};
}

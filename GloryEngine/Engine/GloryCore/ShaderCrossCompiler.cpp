#include "ShaderCrossCompiler.h"

namespace Glory
{
	ShaderCrossCompiler::ShaderCrossCompiler()
	{
	}

	ShaderCrossCompiler::~ShaderCrossCompiler()
	{
	}
	
	void ShaderCrossCompiler::Compile(const uint32_t* data, size_t size)
	{
		spirv_cross::Compiler compiler(data, size);
		compiler.compile();
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();
	}
}

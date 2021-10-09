#pragma once
#include <spirv_cross.hpp>
#include <vector>

namespace Glory
{
	class ShaderCrossCompiler
	{
	public:
		ShaderCrossCompiler();
		virtual ~ShaderCrossCompiler();

		void Compile(const uint32_t* data, size_t size);
	};
}

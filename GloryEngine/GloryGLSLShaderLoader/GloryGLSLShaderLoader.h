#pragma once
#include <ShaderLoaderModule.h>

namespace Glory
{
	class GloryGLSLShaderLoader : public ShaderLoaderModule
	{
	public:
		GloryGLSLShaderLoader();
		virtual ~GloryGLSLShaderLoader();

	private:
		virtual ShaderData* LoadShader(const std::string& path, const ShaderImportSettings& importSettings) override;
	};
}

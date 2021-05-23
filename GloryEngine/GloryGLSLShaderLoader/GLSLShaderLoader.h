#pragma once
#include <ShaderLoaderModule.h>
#include <shaderc.hpp>

namespace Glory
{
	class GLSLShaderLoader : public ShaderLoaderModule
	{
	public:
		GLSLShaderLoader();
		virtual ~GLSLShaderLoader();

	private:
		virtual ShaderData* LoadShader(const std::string& path, const ShaderImportSettings& importSettings) override;
		//virtual void ProcessOtherLine(ShaderData* pShaderData, const std::string& path, int lineIndex, std::string& line) override;

	private:
		std::string LoadFile(const std::string& path);
		void LoadCache(std::ifstream& file, std::vector<uint32_t>& data);
		std::string GetCachePath(const std::string& source);
		void GetOrCreateSpirVShaderCache(const std::string& path, const ShaderImportSettings& importSettings, std::vector<uint32_t>& data);
		void CreateSpirVShaderCache(const std::string& path, const std::string& cachePath, const std::string& source, const ShaderImportSettings& importSettings, std::vector<uint32_t>& data);
		shaderc_shader_kind GetShaderSourceKind(const std::string& path);

	private:
		static const std::string SHADERCACHEPATH;
	};
}

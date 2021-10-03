#pragma once
#include "ResourceLoaderModule.h"
#include "ShaderData.h"
#include <vector>
#include <map>
#include <functional>

namespace Glory
{
	struct ShaderImportSettings : ImportSettings
	{
		std::vector<std::string> CompilerDefinitions;
	};

	class ShaderLoaderModule : public ResourceLoaderModule<ShaderData, ShaderImportSettings>
	{
	public:
		ShaderLoaderModule();
		virtual ~ShaderLoaderModule();

		virtual const std::type_info& GetModuleType() override;

	protected:
		virtual ShaderData* LoadShader(const std::string& path, const ShaderImportSettings& importSettings) = 0;
		std::vector<uint32_t>& GetData(ShaderData* pShaderData);

	private:
		virtual ShaderData* LoadResource(const std::string& path, const ShaderImportSettings& importSettings) override;

	protected:
		virtual void Initialize() override {};
		virtual void Cleanup() override {};
	};
}

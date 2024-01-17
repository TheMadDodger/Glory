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
		ShaderImportSettings();
		ShaderImportSettings(const std::string& extension);

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

		virtual ShaderImportSettings ReadImportSettings_Internal(YAML::Node& node) override;
		virtual void WriteImportSettings_Internal(const ShaderImportSettings& importSettings, YAML::Emitter& out) override;

	private:
		virtual ShaderData* LoadResource(const std::string& path, const ShaderImportSettings& importSettings) override;
		virtual ShaderData* LoadResource(const void* buffer, size_t length, const ShaderImportSettings& importSettings) override;

	protected:
		virtual void Initialize() override { ResourceLoaderModule::Initialize(); };
		virtual void Cleanup() override {};
	};
}

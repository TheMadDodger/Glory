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

	};

	class ShaderLoaderModule : public ResourceLoaderModule<ShaderData, ShaderImportSettings>
	{
	public:
		ShaderLoaderModule();
		virtual ~ShaderLoaderModule();

		virtual const std::type_info& GetModuleType() override;

	protected:
		virtual ShaderData* LoadShader(const std::string& path, const ShaderImportSettings& importSettings) = 0;

	private:
		virtual ShaderData* LoadResource(const std::string& path, const ShaderImportSettings& importSettings) override;

	private:
		virtual bool IncludeDirectiveCallback(const std::string& directive, const std::string& );

	protected:
		virtual void Initialize() override;
		virtual void Cleanup() = 0;
		virtual void Update() override {};
		virtual void Draw() override {};

	private:
		std::map<std::string, std::function<bool(const std::string&, const std::string&)>> m_DirectiveCallbacks;
	};
}

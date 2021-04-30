#include "ShaderLoaderModule.h"

namespace Glory
{
	ShaderLoaderModule::ShaderLoaderModule()
	{
	}

	ShaderLoaderModule::~ShaderLoaderModule()
	{
	}

	const std::type_info& ShaderLoaderModule::GetModuleType()
	{
		return typeid(ShaderLoaderModule);
	}

	ShaderData* ShaderLoaderModule::LoadResource(const std::string& path, const ShaderImportSettings& importSettings)
	{
		ShaderData* pShaderData = LoadShader(path, importSettings);
		//pTexture->BuildTexture();
		return pShaderData;
	}

	void ShaderLoaderModule::Initialize()
	{
		m_DirectiveCallbacks["include"] = [&](const std::string& directive, const std::string& data) { return IncludeDirectiveCallback(directive, data); };
	}

	bool ShaderLoaderModule::IncludeDirectiveCallback(const std::string& directive, const std::string&)
	{
		return false;
	}
}

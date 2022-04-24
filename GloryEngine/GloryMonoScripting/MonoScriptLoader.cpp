#include "MonoScriptLoader.h"
#include <Engine.h>

namespace Glory
{
	MonoScriptLoader::MonoScriptLoader() : ScriptLoaderModule(".cs")
	{
	}

	MonoScriptLoader::~MonoScriptLoader()
	{
	}

	ScriptImportSettings MonoScriptLoader::ReadImportSettings_Internal(YAML::Node& node)
	{
		return ScriptImportSettings();
	}

	void MonoScriptLoader::WriteImportSettings_Internal(const ScriptImportSettings& importSettings, YAML::Emitter& out)
	{
	}

	void MonoScriptLoader::Initialize()
	{
	}

	void MonoScriptLoader::Cleanup()
	{
	}

	MonoScript* MonoScriptLoader::LoadResource(const std::string& path, const ScriptImportSettings& importSettings)
	{
		return nullptr;
	}

	MonoScript* MonoScriptLoader::LoadResource(const void* buffer, size_t length, const ScriptImportSettings& importSettings)
	{
		return nullptr;
	}

	void MonoScriptLoader::SaveResource(const std::string& path, MonoScript* pResource)
	{
		m_pEngine->GetLoaderModule<FileData>()->Save(path, pResource);
	}
}

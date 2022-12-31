#include "MonoScriptLoader.h"
#include <Engine.h>
#include <FileLoaderModule.h>

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
		LoaderModule* pModule = m_pEngine->GetLoaderModule<FileData>();
		FileImportSettings fileImportSettings;
		fileImportSettings.m_Extension = "cs";
		fileImportSettings.Flags = std::ios::ate;
		FileData* pFileData = (FileData*)pModule->Load(path, fileImportSettings);
		MonoScript* pMonoScript = new MonoScript(pFileData);
		delete pFileData;

		std::string namespaceString = "namespace";
		if (pMonoScript->Size() <= 0) return pMonoScript;
		std::string text = pMonoScript->Data();
		pMonoScript->m_NamespaceName = Find(text, "namespace");
		pMonoScript->m_ClassName = Find(text, "class");

		return pMonoScript;
	}

	MonoScript* MonoScriptLoader::LoadResource(const void* buffer, size_t length, const ScriptImportSettings& importSettings)
	{
		LoaderModule* pModule = m_pEngine->GetLoaderModule<FileData>();
		FileImportSettings fileImportSettings;
		fileImportSettings.m_Extension = "cs";
		fileImportSettings.Flags = std::ios::ate;
		FileData* pFileData = (FileData*)pModule->Load(buffer, length, fileImportSettings);
		MonoScript* pMonoScript = new MonoScript(pFileData);
		delete pFileData;

		return pMonoScript;
	}

	void MonoScriptLoader::SaveResource(const std::string& path, MonoScript* pResource)
	{
		//m_pEngine->GetLoaderModule<FileData>()->Save(path, pResource);
	}

	std::string MonoScriptLoader::Find(const std::string& source, const std::string& toFind)
	{
		size_t startIndex = source.find(toFind);
		if (startIndex == std::string::npos) return "";
		startIndex += toFind.length() + 1;
		size_t endIndex = source.find('\n', startIndex);
		size_t endIndex2 = source.find('{', startIndex);
		size_t endIndex3 = source.find(' ', startIndex);
		size_t actualIndex = endIndex < endIndex2 ? endIndex : endIndex2;
		actualIndex = actualIndex < endIndex3 ? actualIndex : endIndex3;
		if (actualIndex == std::string::npos) return "";
		size_t length = actualIndex - startIndex;
		return source.substr(startIndex, length);
	}
}

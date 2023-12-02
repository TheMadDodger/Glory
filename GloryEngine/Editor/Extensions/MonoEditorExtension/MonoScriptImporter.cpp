#include "MonoScriptImporter.h"

#include <Engine.h>
#include <FileLoaderModule.h>

namespace Glory::Editor
{
	std::string_view MonoScriptImporter::Name() const
	{
		return "MonoScript Importer";
	}

	bool MonoScriptImporter::SupportsExtension(const std::filesystem::path& extension) const
	{
		return extension.compare(".cs") == 0;
	}

	std::string_view Find(const std::string_view source, const std::string& toFind)
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

	MonoScript* MonoScriptImporter::LoadResource(const std::filesystem::path& path) const
	{
		/* @todo Replace with a text file importer (but then again there is no need to load the code?) */
		LoaderModule* pModule = Game::GetGame().GetEngine()->GetLoaderModule<FileData>();
		FileImportSettings fileImportSettings;
		fileImportSettings.m_Extension = "cs";
		fileImportSettings.Flags = std::ios::ate;
		FileData* pFileData = (FileData*)pModule->Load(path.string(), fileImportSettings);

		constexpr std::string_view namespaceString = "namespace";
		if (pFileData->Size() <= 0) return new MonoScript(pFileData, "", "");
		const std::string_view text = pFileData->Data();
		const std::string_view ns = Find(text, "namespace");
		const std::string_view cl = Find(text, "class");

		MonoScript* pMonoScript = new MonoScript(pFileData, ns, cl);
		delete pFileData;

		return pMonoScript;
	}
}

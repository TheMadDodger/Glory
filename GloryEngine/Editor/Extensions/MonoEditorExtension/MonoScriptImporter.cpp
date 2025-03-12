#include "MonoScriptImporter.h"

#include <EditorApplication.h>
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
		if (actualIndex == std::string::npos) return source.substr(startIndex);
		size_t length = actualIndex - startIndex;
		return source.substr(startIndex, length);
	}

	std::vector<std::string_view> SplitLines(std::string_view str)
	{
		size_t previousIndex = 0;
		size_t searchIndex = 0;
		std::vector<std::string_view> lines;

		searchIndex = str.find('\n', searchIndex);
		while (searchIndex != std::string_view::npos)
		{
			std::string_view line = str.substr(previousIndex, searchIndex - previousIndex);
			lines.push_back(line);
			++searchIndex;
			previousIndex = searchIndex;
			searchIndex = str.find("\n", searchIndex);
		}

		std::string_view finalLine = str.substr(previousIndex);
		lines.push_back(finalLine);
		return lines;
	}

	ImportedResource MonoScriptImporter::LoadResource(const std::filesystem::path& path, void*) const
	{
		/* @todo Replace with a text file importer (but then again there is no need to load the code?) */
		LoaderModule* pModule = EditorApplication::GetInstance()->GetEngine()->GetLoaderModule<FileData>();
		FileImportSettings fileImportSettings;
		fileImportSettings.m_Extension = "cs";
		fileImportSettings.Flags = std::ios::ate;
		FileData* pFileData = (FileData*)pModule->Load(path.string(), fileImportSettings);

		constexpr std::string_view namespaceString = "namespace";
		if (pFileData->Size() <= 0) return { path, new MonoScript(pFileData, {}) };
		const std::string_view text = pFileData->Data();

		const auto lines = SplitLines(text);
		std::string_view currentNS = "";

		std::vector<std::string> classes;
		for (size_t i = 0; i < lines.size(); ++i)
		{
			const std::string_view ns = Find(lines[i], "namespace");
			if (ns.size()) currentNS = ns;
			const std::string_view cl = Find(lines[i], "class");
			if (cl.size())
			{
				classes.push_back(std::string(currentNS) + "." + std::string(cl));
			}
		}

		MonoScript* pMonoScript = new MonoScript(pFileData, std::move(classes));
		delete pFileData;

		return { path, pMonoScript };
	}

	bool MonoScriptImporter::SaveResource(const std::filesystem::path& path, MonoScript* pScript) const
	{
		LoaderModule* pModule = EditorApplication::GetInstance()->GetEngine()->GetLoaderModule<FileData>();
		pModule->Save(path.string(), pScript);
		return true;
	}
}

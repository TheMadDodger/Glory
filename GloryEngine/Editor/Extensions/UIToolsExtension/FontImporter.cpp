#include "FontImporter.h"

#include <EditorApplication.h>
#include <Engine.h>
#include <FileLoaderModule.h>

namespace Glory::Editor
{
	std::string_view FontImporter::Name() const
	{
		return "Font Importer";
	}

	bool FontImporter::SupportsExtension(const std::filesystem::path& extension) const
	{
		return extension.compare(".otf") == 0 || extension.compare(".ttf") == 0;
	}

	ImportedResource FontImporter::LoadResource(const std::filesystem::path& path, void*) const
	{
		/* @todo Replace with a text file importer (but then again there is no need to load the code?) */
		LoaderModule* pModule = EditorApplication::GetInstance()->GetEngine()->GetLoaderModule<FileData>();
		FileImportSettings fileImportSettings;
		fileImportSettings.m_Extension = "ttf";
		fileImportSettings.Flags = std::ios::ate;
		FileData* pFileData = (FileData*)pModule->Load(path.string(), fileImportSettings);

		FontData* pFont = new FontData(pFileData);
		delete pFileData;

		return { path, pFont };
	}
}

#include "TextImporter.h"
#include "EditorApplication.h"

#include <Engine.h>
#include <Debug.h>

#include <fstream>

namespace Glory::Editor
{
	TextImporter::TextImporter()
	{
	}

	TextImporter::~TextImporter()
	{
	}

	std::string_view TextImporter::Name() const
	{
		return "Internal Text Importer";
	}

	void TextImporter::Initialize()
	{
	}

	void TextImporter::Cleanup()
	{
	}

	bool TextImporter::SupportsExtension(const std::filesystem::path& extension) const
	{
		return extension.compare(".txt") == 0;
	}

	ImportedResource TextImporter::LoadResource(const std::filesystem::path& path, void*) const
	{
		std::vector<char> buffer;
		if (!ReadFile(path, buffer))
			return nullptr;

		TextFileData* pFile = new TextFileData(std::move(buffer));
		return { path, pFile };
	}

	bool TextImporter::SaveResource(const std::filesystem::path& path, TextFileData* pResource) const
	{
		std::ofstream file(path);

		if (!file.is_open())
		{
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogError("Could not open file: " + path.string());
			return false;
		}

		file.write(pResource->Data(), pResource->Size() - 1);
		file.close();
		return true;
	}

	bool TextImporter::ReadFile(const std::filesystem::path& path, std::vector<char>& buffer) const
	{
		std::ifstream file(path, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogError("Could not open file: " + path.string());
			return false;
		}

		size_t fileSize = (size_t)file.tellg();
		buffer.resize(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		buffer.push_back('\0');
		file.close();
		return true;
	}
}

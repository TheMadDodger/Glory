#include "GenericAudioImporter.h"

#include <EditorApplication.h>
#include <sstream>
#include <Debug.h>

#include <fstream>

namespace Glory::Editor
{
	constexpr size_t NumSupportedExtensions = 6;
	constexpr std::string_view SupportedExtensions[NumSupportedExtensions] = {
		".mp3",
		".MP3",
		".wav",
		".WAV",
		".ogg",
		".OGG",
	};

	GenericAudioImporter::GenericAudioImporter() : m_InitializedFlags(0)
	{
	}

	GenericAudioImporter::~GenericAudioImporter()
	{
	}

	std::string_view GenericAudioImporter::Name() const
	{
		return "Generic Audio Importer";
	}

	bool GenericAudioImporter::SupportsExtension(const std::filesystem::path& extension) const
	{
		for (size_t i = 0; i < NumSupportedExtensions; ++i)
		{
			if (extension.compare(SupportedExtensions[i]) != 0) continue;
			return true;
		}
		return false;
	}

	ImportedResource GenericAudioImporter::LoadResource(const std::filesystem::path& path, void* userData) const
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();

		std::ifstream file(path, std::ios::binary | std::ios::ate);

		if (!file.is_open())
		{
			pEngine->GetDebug().LogError("Could not open file: " + path.string());
			return false;
		}

		std::vector<char> buffer;

		size_t fileSize = (size_t)file.tellg();
		buffer.resize(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		AudioData* pData = new AudioData(std::move(buffer));
		return { path, pData };
	}

	void GenericAudioImporter::Initialize()
	{
	}

	void GenericAudioImporter::Cleanup()
	{
	}
}

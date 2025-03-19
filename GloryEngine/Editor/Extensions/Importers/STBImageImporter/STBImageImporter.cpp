#include "STBImageImporter.h"

#include <EditorApplication.h>

#include <Debug.h>
#include <TextureData.h>

#include <stb_image.h>
#include <sstream>

namespace Glory::Editor
{
	constexpr size_t NumSupportedExtensions = 1;
	constexpr std::string_view SupportedExtensions[NumSupportedExtensions] = {
		".hdr",
	};

	STBHDRImageImporter::STBHDRImageImporter()
	{
	}

	STBHDRImageImporter::~STBHDRImageImporter()
	{
	}

	std::string_view STBHDRImageImporter::Name() const
	{
		return "STBImage HDR Importer";
	}

	bool STBHDRImageImporter::SupportsExtension(const std::filesystem::path& extension) const
	{
		for (size_t i = 0; i < NumSupportedExtensions; ++i)
		{
			if (extension.compare(SupportedExtensions[i]) != 0) continue;
			return true;
		}
		return false;
	}

	ImportedResource STBHDRImageImporter::LoadResource(const std::filesystem::path& path, void* userData) const
	{
		stbi_set_flip_vertically_on_load(true);
		int width, height, nrComponents;
		float* imageData = stbi_loadf(path.string().data(), &width, &height, &nrComponents, 0);

		if (!imageData)
		{
			const char* error = stbi_failure_reason();
			std::stringstream str;
			str << "STB could not load image " << path << " Reason: " << error;
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogWarning(str.str());
			return nullptr;
		}

		const size_t dataSize = width*height*nrComponents*sizeof(float);
		char* data = new char[dataSize];
		std::memcpy(data, imageData, dataSize);
		stbi_image_free(imageData);

		HDRImageData* pData = new HDRImageData(width, height, std::move(data), dataSize);
		ImportedResource importedResource{ path, pData };

		TextureData* pDefualtTexture = new TextureData(pData);
		importedResource.AddChild(pDefualtTexture, "Default");

		return importedResource;
	}

	ImportedResource STBHDRImageImporter::LoadResource(void* data, size_t dataSize, void* userData) const
	{
		return nullptr;
	}
}

#include "STBImageImporter.h"

#include <EditorApplication.h>
#include <EditorAssetDatabase.h>

#include <Debug.h>
#include <TextureData.h>
#include <CubemapData.h>

#include <HdriToCubemap.hpp>
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

	ImageData* GenerateImageData(const std::filesystem::path& path, const std::filesystem::path& subPath, uint32_t resolution, uint32_t numChannels, float* pixels)
	{
		const size_t dataSize = resolution*resolution*numChannels*sizeof(float);
		char* data = new char[dataSize];
		std::memcpy(data, pixels, dataSize);

		PixelFormat internalFormat;
		PixelFormat format;
		switch (numChannels)
		{
		case 3:
			internalFormat = PixelFormat::PF_R16G16B16Sfloat;
			format = PixelFormat::PF_RGB;
			break;
		case 4:
			internalFormat = PixelFormat::PF_R16G16B16A16Sfloat;
			format = PixelFormat::PF_RGBA;
			break;
		default:
			break;
		}

		ImageData* pImage = new ImageData(resolution, resolution, internalFormat, format, numChannels*sizeof(float), std::move(data), dataSize, false, DataType::DT_Float);
		const UUID imageID = EditorAssetDatabase::ReserveAssetUUID(path.string(), subPath).first;
		pImage->SetResourceUUID(imageID);
		return pImage;
	}

	ImportedResource STBHDRImageImporter::LoadResource(const std::filesystem::path& path, void* userData) const
	{
		/* Load HDRI */
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

		const size_t dataSize = width * height * nrComponents * sizeof(float);
		char* data = new char[dataSize];
		std::memcpy(data, imageData, dataSize);
		stbi_image_free(imageData);

		HDRImageData* pData = new HDRImageData(width, height, std::move(data), dataSize);
		ImportedResource importedResource{ path, pData };

		TextureData* pDefualtTexture = new TextureData(pData);
		importedResource.AddChild(pDefualtTexture, "Default");

		/* Convert to cubemap */
		const uint32_t cubemapResolution = 1024;
		bool linearFilter = false;

		HdriToCubemap<float> hdriToCube_hdr(path.string(), int(cubemapResolution), linearFilter);
		const uint32_t numChanels = uint32_t(hdriToCube_hdr.getNumChannels());
		if (numChanels == 0) return importedResource;

		ImageData* pFront = GenerateImageData(path, "Front", cubemapResolution, numChanels, hdriToCube_hdr.getFront());
		ImageData* pBack = GenerateImageData(path, "Back", cubemapResolution, numChanels, hdriToCube_hdr.getBack());
		ImageData* pUp = GenerateImageData(path, "Up", cubemapResolution, numChanels, hdriToCube_hdr.getUp());
		ImageData* pDown = GenerateImageData(path, "Down", cubemapResolution, numChanels, hdriToCube_hdr.getDown());
		ImageData* pLeft = GenerateImageData(path, "Left", cubemapResolution, numChanels, hdriToCube_hdr.getLeft());
		ImageData* pRight = GenerateImageData(path, "Right", cubemapResolution, numChanels, hdriToCube_hdr.getRight());

		TextureData* pFrontTexture = new TextureData(pFront);
		TextureData* pBackTexture = new TextureData(pBack);
		TextureData* pUpTexture = new TextureData(pUp);
		TextureData* pDownTexture = new TextureData(pDown);
		TextureData* pLeftTexture = new TextureData(pLeft);
		TextureData* pRightTexture = new TextureData(pRight);

		importedResource.AddChild(pFront, "Front").AddChild(pFrontTexture, "Default");
		importedResource.AddChild(pBack, "Back").AddChild(pBackTexture, "Default");
		importedResource.AddChild(pUp, "Up").AddChild(pUpTexture, "Default");
		importedResource.AddChild(pDown, "Down").AddChild(pDownTexture, "Default");
		importedResource.AddChild(pLeft, "Left").AddChild(pLeftTexture, "Default");
		importedResource.AddChild(pRight, "Right").AddChild(pRightTexture, "Default");

		CubemapData* pCubemap = new CubemapData(pRight, pLeft, pDown, pUp, pFront, pBack);
		SamplerSettings& sampler = pCubemap->GetSamplerSettings();
		sampler.MipmapMode = Filter::F_None;
		sampler.AddressModeU = SamplerAddressMode::SAM_ClampToEdge;
		sampler.AddressModeV = SamplerAddressMode::SAM_ClampToEdge;
		sampler.AddressModeW = SamplerAddressMode::SAM_ClampToEdge;
		importedResource.AddChild(pCubemap, "Cubemap");

		return importedResource;
	}

	ImportedResource STBHDRImageImporter::LoadResource(void* data, size_t dataSize, void* userData) const
	{
		return nullptr;
	}
}

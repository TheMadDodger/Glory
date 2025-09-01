#include "TextureImporter.h"

#include <NodeRef.h>

namespace Glory::Editor
{
	TextureImporter::TextureImporter()
	{
	}

	TextureImporter::~TextureImporter()
	{
	}

	std::string_view TextureImporter::Name() const
	{
		return "Internal Texture Importer";
	}

	bool TextureImporter::SupportsExtension(const std::filesystem::path& extension) const
	{
		return extension.compare(".gtex") == 0;
	}

	void TextureImporter::LoadIntoTexture(Utils::YAMLFileRef file, TextureData* pTexture)
	{
		const UUID imageUUID = file["Image"].As<uint64_t>();
		pTexture->Image().SetUUID(imageUUID);

		Utils::NodeValueRef sampler = file["Sampler"];
		pTexture->GetSamplerSettings().MagFilter = sampler["MinFilter"].AsEnum<Filter>(DefaultSampler.MagFilter);
		pTexture->GetSamplerSettings().MinFilter = sampler["MagFilter"].AsEnum<Filter>(DefaultSampler.MinFilter);
		pTexture->GetSamplerSettings().AddressModeU = sampler["AddressModeU"].AsEnum<SamplerAddressMode>(DefaultSampler.AddressModeU);
		pTexture->GetSamplerSettings().AddressModeV = sampler["AddressModeV"].AsEnum<SamplerAddressMode>(DefaultSampler.AddressModeV);
		pTexture->GetSamplerSettings().AddressModeW = sampler["AddressModeW"].AsEnum<SamplerAddressMode>(DefaultSampler.AddressModeW);
		pTexture->GetSamplerSettings().MaxAnisotropy = sampler["MaxAnisotropy"].As<float>(DefaultSampler.MaxAnisotropy);
		pTexture->GetSamplerSettings().UnnormalizedCoordinates = sampler["UnnormalizedCoordinates"].As<bool>(DefaultSampler.UnnormalizedCoordinates);
		pTexture->GetSamplerSettings().CompareEnable = sampler["CompareEnable"].As<bool>(DefaultSampler.CompareEnable);
		pTexture->GetSamplerSettings().CompareOp = sampler["CompareOp"].AsEnum<CompareOp>(DefaultSampler.CompareOp);
		pTexture->GetSamplerSettings().MipmapMode = sampler["MipmapMode"].AsEnum<Filter>(DefaultSampler.MipmapMode);
		pTexture->GetSamplerSettings().MipLODBias = sampler["MipLODBias"].As<float>(DefaultSampler.MipLODBias);
		pTexture->GetSamplerSettings().MinLOD = sampler["MinLOD"].As<float>(DefaultSampler.MinLOD);
		pTexture->GetSamplerSettings().MaxLOD = sampler["MaxLOD"].As<float>(DefaultSampler.MaxLOD);
	}

	ImportedResource TextureImporter::LoadResource(const std::filesystem::path& path, void*) const
	{
		Utils::YAMLFileRef file{ path };
		file.Load();

		TextureData* pTexture = new TextureData();
		LoadIntoTexture(file, pTexture);
		return ImportedResource{ path, pTexture };
	}

	bool TextureImporter::SaveResource(const std::filesystem::path& path, TextureData* pResource) const
	{
		Utils::YAMLFileRef file{ path };
		file["Image"].Set((uint64_t)pResource->Image().AssetUUID());
		Utils::NodeValueRef sampler = file["Sampler"];

		sampler["MinFilter"].SetEnum<Filter>(pResource->GetSamplerSettings().MagFilter);
		sampler["MagFilter"].SetEnum<Filter>(pResource->GetSamplerSettings().MinFilter);
		sampler["AddressModeU"].SetEnum<SamplerAddressMode>(pResource->GetSamplerSettings().AddressModeU);
		sampler["AddressModeV"].SetEnum<SamplerAddressMode>(pResource->GetSamplerSettings().AddressModeV);
		sampler["AddressModeW"].SetEnum<SamplerAddressMode>(pResource->GetSamplerSettings().AddressModeW);
		sampler["MaxAnisotropy"].Set(pResource->GetSamplerSettings().MaxAnisotropy);
		sampler["UnnormalizedCoordinates"].Set(pResource->GetSamplerSettings().UnnormalizedCoordinates);
		sampler["CompareEnable"].Set(pResource->GetSamplerSettings().CompareEnable);
		sampler["CompareOp"].SetEnum<CompareOp>(pResource->GetSamplerSettings().CompareOp);
		sampler["MipmapMode"].SetEnum<Filter>(pResource->GetSamplerSettings().MipmapMode);
		sampler["MipLODBias"].Set(pResource->GetSamplerSettings().MipLODBias);
		sampler["MinLOD"].Set(pResource->GetSamplerSettings().MinLOD);
		sampler["MaxLOD"].Set(pResource->GetSamplerSettings().MaxLOD);

		file.Save();
		return true;
	}

	void TextureImporter::Initialize()
	{
	}

	void TextureImporter::Cleanup()
	{
	}
}
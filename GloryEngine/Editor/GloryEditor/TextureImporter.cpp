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

	TextureData* TextureImporter::LoadResource(const std::filesystem::path& path) const
	{
		Utils::YAMLFileRef file{ path };
		file.Load();

		const UUID imageUUID = file["Image"].As<uint64_t>();

		TextureData* pTexture = new TextureData();
		pTexture->Image().SetUUID(imageUUID);

		Utils::NodeValueRef sampler = file["Sampler"];
		pTexture->GetSamplerSettings().MagFilter = sampler["MinFilter"].AsEnum<Filter>();
		pTexture->GetSamplerSettings().MinFilter = sampler["MagFilter"].AsEnum<Filter>();
		pTexture->GetSamplerSettings().AddressModeU = sampler["AddressModeU"].AsEnum<SamplerAddressMode>();
		pTexture->GetSamplerSettings().AddressModeV = sampler["AddressModeV"].AsEnum<SamplerAddressMode>();
		pTexture->GetSamplerSettings().AddressModeW = sampler["AddressModeW"].AsEnum<SamplerAddressMode>();
		pTexture->GetSamplerSettings().AnisotropyEnable = sampler["AnisotropyEnable"].As<bool>();
		pTexture->GetSamplerSettings().MaxAnisotropy = sampler["MaxAnisotropy"].As<float>();
		pTexture->GetSamplerSettings().UnnormalizedCoordinates = sampler["UnnormalizedCoordinates"].As<bool>();
		pTexture->GetSamplerSettings().CompareEnable = sampler["CompareEnable"].As<bool>();
		pTexture->GetSamplerSettings().CompareOp = sampler["CompareOp"].AsEnum<CompareOp>();
		pTexture->GetSamplerSettings().MipmapMode = sampler["MipmapMode"].AsEnum<Filter>();
		pTexture->GetSamplerSettings().MipLODBias = sampler["MipLODBias"].As<float>();
		pTexture->GetSamplerSettings().MinLOD = sampler["MinLOD"].As<float>();
		pTexture->GetSamplerSettings().MaxLOD = sampler["MaxLOD"].As<float>();

		return pTexture;
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
		sampler["AnisotropyEnable"].Set(pResource->GetSamplerSettings().AnisotropyEnable);
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
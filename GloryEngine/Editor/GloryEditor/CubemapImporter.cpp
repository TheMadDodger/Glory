#include "CubemapImporter.h"

namespace Glory::Editor
{
	CubemapImporter::CubemapImporter()
	{
	}

	CubemapImporter::~CubemapImporter()
	{
	}

	std::string_view CubemapImporter::Name() const
	{
		return "Internal Cubemap Importer";
	}

	bool CubemapImporter::SupportsExtension(const std::filesystem::path& extension) const
	{
		return extension.compare(".gcube") == 0;
	}

	ImportedResource CubemapImporter::LoadResource(const std::filesystem::path& path, void* userData) const
	{
		static constexpr std::string_view sides[6] = {
			"Right",
			"Left",
			"Down",
			"Up",
			"Front",
			"Back",
		};

		Utils::YAMLFileRef file{ path };
		auto root = file.RootNodeRef().ValueRef();
		if (!root.IsMap())
		{
			return ImportedResource(path, new CubemapData());
		}

		auto faces = root["Faces"];
		UUID faceIDs[6];
		for (size_t i = 0; i < 6; ++i)
		{
			auto side = faces[sides[i]];
			faceIDs[i] = side.As<uint64_t>(0);
		}
		CubemapData* pCubemap = new CubemapData(faceIDs[0], faceIDs[1], faceIDs[2], faceIDs[3], faceIDs[4], faceIDs[5]);
		SamplerSettings& samplerSettings = pCubemap->GetSamplerSettings();

		Utils::NodeValueRef sampler = root["Sampler"];
		samplerSettings.MagFilter = sampler["MinFilter"].AsEnum<Filter>(DefaultSampler.MagFilter);
		samplerSettings.MinFilter = sampler["MagFilter"].AsEnum<Filter>(DefaultSampler.MinFilter);
		samplerSettings.AddressModeU = sampler["AddressModeU"].AsEnum<SamplerAddressMode>(DefaultSampler.AddressModeU);
		samplerSettings.AddressModeV = sampler["AddressModeV"].AsEnum<SamplerAddressMode>(DefaultSampler.AddressModeV);
		samplerSettings.AddressModeW = sampler["AddressModeW"].AsEnum<SamplerAddressMode>(DefaultSampler.AddressModeW);
		samplerSettings.MaxAnisotropy = sampler["MaxAnisotropy"].As<float>(DefaultSampler.MaxAnisotropy);
		samplerSettings.UnnormalizedCoordinates = sampler["UnnormalizedCoordinates"].As<bool>(DefaultSampler.UnnormalizedCoordinates);
		samplerSettings.CompareEnable = sampler["CompareEnable"].As<bool>(DefaultSampler.CompareEnable);
		samplerSettings.CompareOp = sampler["CompareOp"].AsEnum<CompareOp>(DefaultSampler.CompareOp);
		samplerSettings.MipmapMode = sampler["MipmapMode"].AsEnum<Filter>(DefaultSampler.MipmapMode);
		samplerSettings.MipLODBias = sampler["MipLODBias"].As<float>(DefaultSampler.MipLODBias);
		samplerSettings.MinLOD = sampler["MinLOD"].As<float>(DefaultSampler.MinLOD);
		samplerSettings.MaxLOD = sampler["MaxLOD"].As<float>(DefaultSampler.MaxLOD);

		return ImportedResource(path, pCubemap);
	}

	ImportedResource CubemapImporter::LoadResource(void* data, size_t dataSize, void* userData) const
	{
		return nullptr;
	}

	bool CubemapImporter::SaveResource(const std::filesystem::path& path, CubemapData* pResource) const
	{
		static constexpr std::string_view sides[6] = {
			"Right",
			"Left",
			"Down",
			"Up",
			"Front",
			"Back",
		};

		Utils::YAMLFileRef file{ path };
		auto root = file.RootNodeRef().ValueRef();
		root.SetMap();
		auto faces = root["Faces"];
		faces.SetMap();

		for (size_t i = 0; i < 6; ++i)
		{
			auto face = faces[sides[i]];
			face.Set(uint64_t(pResource->Image(i).AssetUUID()));
		}

		Utils::NodeValueRef sampler = root["Sampler"];
		SamplerSettings& samplerSettings = pResource->GetSamplerSettings();
		sampler["MinFilter"].SetEnum<Filter>(samplerSettings.MagFilter);
		sampler["MagFilter"].SetEnum<Filter>(samplerSettings.MinFilter);
		sampler["AddressModeU"].SetEnum<SamplerAddressMode>(samplerSettings.AddressModeU);
		sampler["AddressModeV"].SetEnum<SamplerAddressMode>(samplerSettings.AddressModeV);
		sampler["AddressModeW"].SetEnum<SamplerAddressMode>(samplerSettings.AddressModeW);
		sampler["MaxAnisotropy"].Set(samplerSettings.MaxAnisotropy);
		sampler["UnnormalizedCoordinates"].Set(samplerSettings.UnnormalizedCoordinates);
		sampler["CompareEnable"].Set(samplerSettings.CompareEnable);
		sampler["CompareOp"].SetEnum<CompareOp>(samplerSettings.CompareOp);
		sampler["MipmapMode"].SetEnum<Filter>(samplerSettings.MipmapMode);
		sampler["MipLODBias"].Set(samplerSettings.MipLODBias);
		sampler["MinLOD"].Set(samplerSettings.MinLOD);
		sampler["MaxLOD"].Set(samplerSettings.MaxLOD);

		file.Save();
		return true;
	}
}

#include "PipelineImporter.h"

#include <YAML_GLM.h>

namespace Glory::Editor
{
	PipelineImporter::PipelineImporter()
	{
	}

	PipelineImporter::~PipelineImporter()
	{
	}

	std::string_view PipelineImporter::Name() const
	{
		return "Internal Pipeline Importer";
	}

	bool PipelineImporter::SupportsExtension(const std::filesystem::path& extension) const
	{
		return extension.compare(".gpln") == 0;
	}

	ImportedResource PipelineImporter::LoadResource(const std::filesystem::path& path, void*) const
	{
		PipelineData* pPipeline = new PipelineData();
		Utils::YAMLFileRef file{ path };
		const PipelineType type = file["Type"].AsEnum<PipelineType>();
		pPipeline->SetPipelineType(type);
		for (size_t i = 0; i < file["Shaders"].Size(); ++i)
		{
			const UUID shaderID = file["Shaders"][i].As<uint64_t>();
			pPipeline->AddShader(shaderID);
		}

		auto features = file["Features"];
		if (features.Exists() && features.IsMap())
		{
			for (auto iter = features.Begin(); iter != features.End(); ++iter)
			{
				const std::string name = *iter;
				const bool isOn = features[name].As<bool>();
				pPipeline->AddFeature(name, isOn);
			}
		}

		auto settings = file["Settings"];
		pPipeline->GetCullFace() = settings["CullFace"].AsEnum<CullFace>(CullFace::Back);
		pPipeline->GetPrimitiveType() = settings["PrimitiveType"].AsEnum<PrimitiveType>(PrimitiveType::Triangles);
		pPipeline->SetDepthTestEnabled(settings["DepthTestEnabled"].As<bool>(true));
		pPipeline->SetDepthWriteEnabled(settings["DepthWriteEnabled"].As<bool>(true));
		pPipeline->GetDepthCompareOp() = settings["DepthCompareOp"].AsEnum<CompareOp>(CompareOp::OP_Less);
		pPipeline->SetColorWriteMask(settings["ColorWriteMask"].As<uint8_t>(15));
		pPipeline->SetStencilTestEnabled(settings["StencilTestEnabled"].As<bool>(false));
		pPipeline->GetStencilCompareOp() = settings["StencilCompareOp"].AsEnum<CompareOp>(CompareOp::OP_Always);
		pPipeline->GetStencilFailOp() = settings["StencilFailOp"].AsEnum<Func>(Func::OP_Zero);
		pPipeline->GetStencilDepthFailOp() = settings["StencilDepthFailOp"].AsEnum<Func>(Func::OP_Zero);
		pPipeline->GetStencilPassOp() = settings["StencilPassOp"].AsEnum<Func>(Func::OP_Zero);
		pPipeline->SetStencilCompareMask(settings["StencilCompareMask"].As<uint8_t>(0xFF));
		pPipeline->SetStencilWriteMask(settings["StencilWriteMask"].As<uint8_t>(0x00));
		pPipeline->SetStencilReference(settings["StencilReference"].As<uint8_t>(0x00));

		pPipeline->SetBlendEnabled(settings["BlendEnabled"].As<bool>(true));
		pPipeline->SrcColorBlendFactor() = settings["SrcColorBlendFactor"].AsEnum<BlendFactor>(BlendFactor::One);
		pPipeline->DstColorBlendFactor() = settings["DrcColorBlendFactor"].AsEnum<BlendFactor>(BlendFactor::Zero);
		pPipeline->ColorBlendOp() = settings["ColorBlendOp"].AsEnum<BlendOp>(BlendOp::Add);
		pPipeline->SrcAlphaBlendFactor() = settings["SrcAlphaBlendFactor"].AsEnum<BlendFactor>(BlendFactor::One);
		pPipeline->DstAlphaBlendFactor() = settings["DstAlphaBlendFactor"].AsEnum<BlendFactor>(BlendFactor::Zero);
		pPipeline->AlphaBlendOp() = settings["AlphaBlendOp"].AsEnum<BlendOp>(BlendOp::Add);
		pPipeline->BlendConstants() = settings["BlendConstants"].As<glm::vec4>(glm::vec4{});

		return ImportedResource(path, pPipeline);
	}

	bool PipelineImporter::SaveResource(const std::filesystem::path& path, PipelineData* pResource) const
	{
		Utils::YAMLFileRef file{ path };

		file["Type"].SetEnum(pResource->Type());
		file["Shaders"].Set(YAML::Node(YAML::NodeType::Sequence));

		for (size_t i = 0; i < pResource->ShaderCount(); ++i)
			file["Shaders"].PushBack((uint64_t)pResource->ShaderID(i));

		auto features = file["Features"];
		features.SetMap();

		for (size_t i = 0; i < pResource->FeatureCount(); ++i)
		{
			const std::string_view name = pResource->FeatureName(i);
			const bool isOn = pResource->FeatureEnabled(i);
			features[name].Set(isOn);
		}

		auto settings = file["Settings"];
		if (!settings.Exists() || !settings.IsMap())
			settings.SetMap();
		settings["CullFace"].SetEnum<CullFace>(pResource->GetCullFace());
		settings["PrimitiveType"].SetEnum<PrimitiveType>(pResource->GetPrimitiveType());
		settings["DepthTestEnabled"].Set<bool>(pResource->DepthTestEnabled());
		settings["DepthWriteEnabled"].Set<bool>(pResource->DepthWriteEnabled());
		settings["DepthCompareOp"].SetEnum<CompareOp>(pResource->GetDepthCompareOp());
		settings["ColorWriteMask"].Set<uint32_t>(uint32_t(pResource->ColorWriteMask()));
		settings["StencilTestEnabled"].Set<bool>(pResource->StencilTestEnabled());
		settings["StencilCompareOp"].SetEnum<CompareOp>(pResource->GetStencilCompareOp());
		settings["StencilFailOp"].SetEnum<Func>(pResource->GetStencilFailOp());
		settings["StencilDepthFailOp"].SetEnum<Func>(pResource->GetStencilDepthFailOp());
		settings["StencilPassOp"].SetEnum<Func>(pResource->GetStencilPassOp());
		settings["StencilCompareMask"].Set<uint32_t>(uint32_t(pResource->StencilCompareMask()));
		settings["StencilWriteMask"].Set<uint32_t>(uint32_t(pResource->StencilWriteMask()));
		settings["StencilReference"].Set<uint32_t>(uint32_t(pResource->StencilReference()));

		settings["BlendEnabled"].Set<bool>(pResource->BlendEnabled());
		settings["SrcColorBlendFactor"].SetEnum<BlendFactor>(pResource->SrcColorBlendFactor());
		settings["DrcColorBlendFactor"].SetEnum<BlendFactor>(pResource->DstColorBlendFactor());
		settings["ColorBlendOp"].SetEnum<BlendOp>(pResource->ColorBlendOp());
		settings["SrcAlphaBlendFactor"].SetEnum<BlendFactor>(pResource->SrcAlphaBlendFactor());
		settings["DstAlphaBlendFactor"].SetEnum<BlendFactor>(pResource->DstAlphaBlendFactor());
		settings["AlphaBlendOp"].SetEnum<BlendOp>(pResource->AlphaBlendOp());
		settings["BlendConstants"].As<glm::vec4>(pResource->BlendConstants());

		file.Save();
		return true;
	}

	void PipelineImporter::Initialize()
	{
	}

	void PipelineImporter::Cleanup()
	{
	}
}

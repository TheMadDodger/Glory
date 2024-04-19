#include "PipelineImporter.h"

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

		return ImportedResource(path, pPipeline);
	}

	bool PipelineImporter::SaveResource(const std::filesystem::path& path, PipelineData* pResource) const
	{
		Utils::YAMLFileRef file{ path };

		file["Type"].SetEnum(pResource->Type());
		file["Shaders"].Set(YAML::Node(YAML::NodeType::Sequence));

		for (size_t i = 0; i < pResource->ShaderCount(); ++i)
			file["Shaders"].PushBack((uint64_t)pResource->ShaderID(i));

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

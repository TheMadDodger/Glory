#include "MaterialInstanceImporter.h"
#include "EditorApplication.h"
#include "EditorMaterialManager.h"

#include <fstream>
#include <AssetManager.h>
#include <PropertySerializer.h>

namespace Glory::Editor
{
	MaterialInstanceImporter::MaterialInstanceImporter()
	{
	}

	MaterialInstanceImporter::~MaterialInstanceImporter()
	{
	}

	std::string_view MaterialInstanceImporter::Name() const
	{
		return "Internal Material Instance Importer";
	}

	bool MaterialInstanceImporter::SupportsExtension(const std::filesystem::path& extension) const
	{
		return extension.compare(".gminst") == 0;
	}

	MaterialInstanceData* MaterialInstanceImporter::LoadResource(const std::filesystem::path& path) const
	{
		Utils::YAMLFileRef file{ path };
		MaterialInstanceData* pMaterial = nullptr;
		EditorApplication::GetInstance()->GetMaterialManager().LoadIntoMaterial(file, pMaterial);
		return pMaterial;
	}

	bool MaterialInstanceImporter::SaveResource(const std::filesystem::path& path, MaterialInstanceData* pResource) const
	{
		MaterialInstanceData* pMaterialInstance = dynamic_cast<MaterialInstanceData*>(pResource);
		YAML::Emitter out;
		out << YAML::BeginMap;
		SaveMaterialInstanceData(pMaterialInstance, out);
		out << YAML::EndMap;
		std::ofstream outStream(path);
		outStream << out.c_str();
		outStream.close();
		return true;
	}

	void MaterialInstanceImporter::SaveMaterialInstanceData(MaterialInstanceData* pMaterialData, YAML::Emitter& out) const
	{
		const UUID baseMaterial = pMaterialData->BaseMaterialID();
		YAML_WRITE(out, BaseMaterial, baseMaterial);

		MaterialManager& manager = EditorApplication::GetInstance()->GetEngine()->GetMaterialManager();

		out << YAML::Key << "Overrides";
		out << YAML::Value << YAML::BeginMap;
		out << YAML::EndMap;
	}

	void MaterialInstanceImporter::Initialize()
	{
	}

	void MaterialInstanceImporter::Cleanup()
	{
	}
}
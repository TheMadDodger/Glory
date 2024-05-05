#include "MaterialImporter.h"
#include "EditorAssetDatabase.h"
#include "EditorApplication.h"
#include "EditorMaterialManager.h"
#include "EditorResourceManager.h"

#include <fstream>
#include <AssetManager.h>
#include <AssetDatabase.h>
#include <PropertySerializer.h>

namespace Glory::Editor
{
	MaterialImporter::MaterialImporter()
	{
	}

	MaterialImporter::~MaterialImporter()
	{
	}

	std::string_view MaterialImporter::Name() const
	{
		return "Internal Material Importer";
	}

	bool MaterialImporter::SupportsExtension(const std::filesystem::path& extension) const
	{
		return extension.compare(".gmat") == 0;
	}

	ImportedResource MaterialImporter::LoadResource(const std::filesystem::path& path, void*) const
	{
		MaterialData* pMaterialData = new MaterialData();
		Utils::YAMLFileRef file{ path };
		EditorApplication::GetInstance()->GetMaterialManager().LoadIntoMaterial(file, pMaterialData);
		return ImportedResource{ path, pMaterialData };
	}

	bool MaterialImporter::SaveResource(const std::filesystem::path& path, MaterialData* pResource) const
	{
		YAML::Emitter out;
		SaveMaterialData(pResource, out);
		std::ofstream outStream(path);
		outStream << out.c_str();
		outStream.close();
		return true;
	}

	void MaterialImporter::SaveMaterialData(MaterialData* pMaterialData, YAML::Emitter& out) const
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Pipeline";
		out << YAML::Value << uint64_t(pMaterialData->GetPipelineID(EditorApplication::GetInstance()->GetMaterialManager()));
		WritePropertyData(out, pMaterialData);
		out << YAML::EndMap;
	}

	void MaterialImporter::WritePropertyData(YAML::Emitter& out, MaterialData* pMaterialData) const
	{
		MaterialManager& manager = EditorApplication::GetInstance()->GetEngine()->GetMaterialManager();

		out << YAML::Key << "Properties";
		out << YAML::Value << YAML::BeginSeq;

		size_t resourceIndex = 0;
		for (size_t i = 0; i < pMaterialData->PropertyInfoCount(manager); i++)
		{
			out << YAML::BeginMap;

			MaterialPropertyInfo* pPropertyInfo = pMaterialData->GetPropertyInfoAt(manager, i);
			std::string name;
			YAML_WRITE(out, DisplayName, pPropertyInfo->DisplayName());
			YAML_WRITE(out, ShaderName, pPropertyInfo->ShaderName());
			YAML_WRITE(out, TypeHash, pPropertyInfo->TypeHash());

			bool isResource = EditorApplication::GetInstance()->GetEngine()->GetResourceTypes().IsResource(pPropertyInfo->TypeHash());
			if (!isResource)
			{
				EditorApplication::GetInstance()->GetEngine()->GetSerializers().SerializeProperty("Value", pMaterialData->GetBufferReference(manager), pPropertyInfo->TypeHash(), pPropertyInfo->Offset(), pPropertyInfo->Size(), out);
			}
			else
			{
				size_t index = pMaterialData->GetPropertyIndexFromResourceIndex(manager, resourceIndex);
				++resourceIndex;
				const UUID uuid = pMaterialData->GetResourceUUIDPointer(manager, index)->AssetUUID();
				out << YAML::Key << "Value" << YAML::Value << uuid;
			}

			out << YAML::EndMap;
		}

		out << YAML::EndSeq;
	}

	void MaterialImporter::Initialize()
	{
	}

	void MaterialImporter::Cleanup()
	{
	}
}

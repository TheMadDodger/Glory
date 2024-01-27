#include "MaterialImporter.h"
#include "EditorAssetDatabase.h"
#include "EditorApplication.h"
#include "EditorMaterialManager.h"

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

	MaterialData* MaterialImporter::LoadResource(const std::filesystem::path& path) const
	{
		MaterialData* pMaterialData = new MaterialData();
		Utils::YAMLFileRef file{ path };
		EditorApplication::GetInstance()->GetMaterialManager().LoadIntoMaterial(file, pMaterialData);
		return pMaterialData;
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
		WriteShaders(out, pMaterialData);
		WritePropertyData(out, pMaterialData);
		out << YAML::EndMap;
	}

	void MaterialImporter::WriteShaders(YAML::Emitter& out, MaterialData* pMaterialData) const
	{
		out << YAML::Key << "Shaders";
		out << YAML::Value << YAML::BeginSeq;
		for (size_t i = 0; i < pMaterialData->ShaderCount(); i++)
		{
			out << YAML::BeginMap;
			ShaderSourceData* pShaderSourceData = pMaterialData->GetShaderAt(i);
			YAML_WRITE(out, UUID, pShaderSourceData->GetUUID());
			YAML_WRITE(out, Type, pShaderSourceData->GetShaderType());
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
	}

	void MaterialImporter::WritePropertyData(YAML::Emitter& out, MaterialData* pMaterialData) const
	{
		out << YAML::Key << "Properties";
		out << YAML::Value << YAML::BeginSeq;

		size_t resourceIndex = 0;
		for (size_t i = 0; i < pMaterialData->PropertyInfoCount(); i++)
		{
			out << YAML::BeginMap;

			MaterialPropertyInfo* pPropertyInfo = pMaterialData->GetPropertyInfoAt(i);
			std::string name;
			YAML_WRITE(out, DisplayName, pPropertyInfo->DisplayName());
			YAML_WRITE(out, ShaderName, pPropertyInfo->ShaderName());
			YAML_WRITE(out, TypeHash, pPropertyInfo->TypeHash());

			bool isResource = EditorApplication::GetInstance()->GetEngine()->GetResourceTypes().IsResource(pPropertyInfo->TypeHash());
			if (!isResource)
			{
				EditorApplication::GetInstance()->GetEngine()->GetSerializers().SerializeProperty("Value", pMaterialData->GetBufferReference(), pPropertyInfo->TypeHash(), pPropertyInfo->Offset(), pPropertyInfo->Size(), out);
			}
			else
			{
				size_t index = pMaterialData->GetPropertyIndexFromResourceIndex(resourceIndex);
				++resourceIndex;
				const UUID uuid = pMaterialData->GetResourceUUIDPointer(index)->AssetUUID();
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

#include "MaterialImporter.h"
#include "EditorAssetDatabase.h"
#include "EditorApplication.h"

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
		YAML::Node rootNode = YAML::LoadFile(path.string());
		return LoadMaterialData(rootNode);
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

	MaterialData* MaterialImporter::LoadMaterialData(YAML::Node& rootNode) const
	{
		MaterialData* pMaterialData = new MaterialData();
		ReadShaders(rootNode, pMaterialData);
		ReadPropertyData(rootNode, pMaterialData);
		return pMaterialData;
	}

	void MaterialImporter::SaveMaterialData(MaterialData* pMaterialData, YAML::Emitter& out) const
	{
		out << YAML::BeginMap;
		WriteShaders(out, pMaterialData);
		WritePropertyData(out, pMaterialData);
		out << YAML::EndMap;
	}

	void MaterialImporter::ReadShaders(YAML::Node& rootNode, MaterialData* pMaterialData) const
	{
		YAML::Node shadersNode = rootNode["Shaders"];
		if (!shadersNode.IsSequence()) return;
		for (size_t i = 0; i < shadersNode.size(); i++)
		{
			YAML::Node shaderNode = shadersNode[i];
			YAML::Node node;
			UUID shaderUUID;
			ShaderType shaderType;
			YAML_READ(shaderNode, node, UUID, shaderUUID, uint64_t);
			YAML_READ(shaderNode, node, Type, shaderType, ShaderType);

			AssetLocation location;
			if (!EditorAssetDatabase::GetAssetLocation(shaderUUID, location)) continue;

			const std::string_view assetPath = EditorApplication::GetInstance()->GetEngine()->GetAssetDatabase().GetAssetPath();
			const std::string path = std::string{ assetPath } + '\\' + location.Path;
			ShaderSourceData* pShaderSourceData = EditorApplication::GetInstance()->GetEngine()->GetAssetManager().GetAssetImmediate<ShaderSourceData>(shaderUUID);
			if (!pShaderSourceData) continue;

			pMaterialData->AddShader(pShaderSourceData);
		}
	}

	void MaterialImporter::ReadPropertyData(YAML::Node& rootNode, MaterialData* pMaterialData) const
	{
		YAML::Node propertiesNode = rootNode["Properties"];
		if (!propertiesNode.IsSequence()) return;

		for (size_t i = 0; i < propertiesNode.size(); i++)
		{
			YAML::Node propertyNode = propertiesNode[i];
			YAML::Node node;
			uint32_t typeHash = 0;
			std::string displayName;
			std::string shaderName;
			YAML_READ(propertyNode, node, DisplayName, displayName, std::string);
			YAML_READ(propertyNode, node, ShaderName, shaderName, std::string);
			YAML_READ(propertyNode, node, TypeHash, typeHash, uint32_t);

			node = propertyNode["Value"];

			const BasicTypeData* typeData = EditorApplication::GetInstance()->GetEngine()->GetResourceTypes().GetBasicTypeData(typeHash);

			size_t offset = pMaterialData->GetCurrentBufferOffset();

			bool isResource = EditorApplication::GetInstance()->GetEngine()->GetResourceTypes().IsResource(typeHash);
			if (!isResource)
			{
				pMaterialData->AddProperty(displayName, shaderName, typeHash, typeData != nullptr ? typeData->m_Size : 4, 0);
				EditorApplication::GetInstance()->GetEngine()->GetSerializers().DeserializeProperty(pMaterialData->GetBufferReference(), typeHash, offset, typeData != nullptr ? typeData->m_Size : 4, node);
			}
			else
			{
				UUID id = node.as<uint64_t>();
				pMaterialData->AddProperty(displayName, shaderName, typeHash, id);
			}
		}
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

#include "MaterialLoaderModule.h"
#include "GLORY_YAML.h"
#include "PropertySerializer.h"
#include "AssetManager.h"
#include "ShaderManager.h"

namespace Glory
{
	MaterialImportSettings::MaterialImportSettings() {}
	MaterialImportSettings::MaterialImportSettings(const std::string& name)
		: ImportSettings(name) {}

	MaterialLoaderModule::MaterialLoaderModule() : ResourceLoaderModule(".gmat")
	{
	}

	MaterialLoaderModule::~MaterialLoaderModule()
	{
	}

	const std::type_info& MaterialLoaderModule::GetModuleType()
	{
		return typeid(MaterialLoaderModule);
	}

	MaterialImportSettings MaterialLoaderModule::ReadImportSettings_Internal(YAML::Node& node)
	{
		return MaterialImportSettings();
	}

	void MaterialLoaderModule::WriteImportSettings_Internal(const MaterialImportSettings& importSettings, YAML::Emitter& out)
	{
	}

	MaterialData* MaterialLoaderModule::LoadResource(const std::string& path, const MaterialImportSettings& importSettings)
	{
		YAML::Node rootNode = YAML::LoadFile(path);
		return LoadMaterialData(rootNode, importSettings);
	}

	MaterialData* MaterialLoaderModule::LoadResource(const void* buffer, size_t length, const MaterialImportSettings& importSettings)
	{
		YAML::Node rootNode = YAML::Load((const char*)buffer);
		return LoadMaterialData(rootNode, importSettings);
	}

	void MaterialLoaderModule::SaveResource(const std::string& path, MaterialData* pResource)
	{
		MaterialInstanceData* pMaterialInstance = dynamic_cast<MaterialInstanceData*>(pResource);
		YAML::Emitter out;
		SaveMaterialData(pResource, out);
		std::ofstream outStream(path);
		outStream << out.c_str();
		outStream.close();
	}

	MaterialData* MaterialLoaderModule::LoadMaterialData(YAML::Node& rootNode, const MaterialImportSettings& importSettings)
	{
		MaterialData* pMaterialData = new MaterialData();
		ReadShaders(rootNode, pMaterialData);
		ReadPropertyData(rootNode, pMaterialData);
		return pMaterialData;
	}

	void MaterialLoaderModule::SaveMaterialData(MaterialData* pMaterialData, YAML::Emitter& out)
	{
		out << YAML::BeginMap;
		WriteShaders(out, pMaterialData);
		WritePropertyData(out, pMaterialData);
		out << YAML::EndMap;
	}
	
	void MaterialLoaderModule::ReadShaders(YAML::Node& rootNode, MaterialData* pMaterialData)
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
			if (!AssetDatabase::GetAssetLocation(shaderUUID, location)) continue;
			
			std::string path = Game::GetAssetPath() + '\\' + location.Path;
			ShaderSourceData* pShaderSourceData = AssetManager::GetAssetImmediate<ShaderSourceData>(shaderUUID);
			if (!pShaderSourceData) continue;

			pMaterialData->m_pShaderFiles.push_back(pShaderSourceData);
		}
	}

	void MaterialLoaderModule::ReadPropertyData(YAML::Node& rootNode, MaterialData* pMaterialData)
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

			const BasicTypeData* typeData = ResourceType::GetBasicTypeData(typeHash);

			size_t offset = pMaterialData->GetCurrentBufferOffset();

			bool isResource = ResourceType::IsResource(typeHash);
			if (!isResource)
			{
				pMaterialData->AddProperty(displayName, shaderName, typeHash, typeData != nullptr ? typeData->m_Size : 4, 0);
				PropertySerializer::DeserializeProperty(pMaterialData->GetBufferReference(), typeHash, offset, typeData != nullptr ? typeData->m_Size : 4, node);
			}
			else
			{
				UUID id = node.as<uint64_t>();
				pMaterialData->AddProperty(displayName, shaderName, typeHash, id);
			}
		}
	}

	void MaterialLoaderModule::WriteShaders(YAML::Emitter& out, MaterialData* pMaterialData)
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

	void MaterialLoaderModule::WritePropertyData(YAML::Emitter& out, MaterialData* pMaterialData)
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

			bool isResource = ResourceType::IsResource(pPropertyInfo->m_TypeHash);
			if (!isResource)
			{
				PropertySerializer::SerializeProperty("Value", pMaterialData->GetBufferReference(), pPropertyInfo->m_TypeHash, pPropertyInfo->m_Offset, pPropertyInfo->m_Size, out);
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

	void MaterialLoaderModule::Initialize()
	{

	}

	void MaterialLoaderModule::Cleanup()
	{

	}
}

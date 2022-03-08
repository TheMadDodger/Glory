#include "MaterialLoaderModule.h"
#include "GLORY_YAML.h"
#include "Serializer.h"
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
		YAML::Node node;
		bool isInstance = false;
		YAML_READ(rootNode, node, IsInstance, isInstance, bool);
		return isInstance ? LoadMaterialInstanceData(rootNode, importSettings) : LoadMaterialData(rootNode, importSettings);
	}

	MaterialData* MaterialLoaderModule::LoadResource(const void* buffer, size_t length, const MaterialImportSettings& importSettings)
	{
		YAML::Node rootNode = YAML::Load((const char*)buffer);
		YAML::Node node;
		bool isInstance = false;
		YAML_READ(rootNode, node, IsInstance, isInstance, bool);
		return isInstance ? LoadMaterialInstanceData(rootNode, importSettings) : LoadMaterialData(rootNode, importSettings);
	}

	void MaterialLoaderModule::SaveResource(const std::string& path, MaterialData* pResource)
	{
		MaterialInstanceData* pMaterialInstance = dynamic_cast<MaterialInstanceData*>(pResource);
		YAML::Emitter out;

		if (pMaterialInstance) SaveMaterialInstanceData(pMaterialInstance, out);
		else SaveMaterialData(pResource, out);

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

	MaterialInstanceData* MaterialLoaderModule::LoadMaterialInstanceData(YAML::Node& rootNode, const MaterialImportSettings& importSettings)
	{
		YAML::Node node;
		UUID baseMaterial = 0;
		YAML_READ(rootNode, node, BaseMaterial, baseMaterial, uint64_t);
		MaterialData* pMaterialData = AssetManager::GetAssetImmediate<MaterialData>(baseMaterial);
		MaterialInstanceData* pMaterialInstanceData = new MaterialInstanceData(pMaterialData);
		ReadPropertyOverrides(rootNode, pMaterialInstanceData);
		return pMaterialInstanceData;
	}

	void MaterialLoaderModule::SaveMaterialData(MaterialData* pMaterialData, YAML::Emitter& out)
	{
		out << YAML::BeginMap;
		WriteShaders(out, pMaterialData);
		out << YAML::EndMap;
	}

	void MaterialLoaderModule::SaveMaterialInstanceData(MaterialInstanceData* pMaterialData, YAML::Emitter& out)
	{

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
			
			std::string path = Game::GetAssetPath() + '\\' + location.m_Path;
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
			std::string name;
			size_t typeHash = 0;
			YAML_READ(propertyNode, node, Name, name, std::string);
			YAML_READ(propertyNode, node, TypeHash, typeHash, size_t);
			
			node = propertyNode["Value"];

			const BasicTypeData* typeData = ResourceType::GetBasicTypeData(typeHash);

			size_t offset = pMaterialData->GetCurrentBufferOffset();

			bool isResource = ResourceType::IsResource(typeHash);
			if (!isResource)
			{
				pMaterialData->AddProperty(name, name, typeHash, typeData != nullptr ? typeData->m_Size : 4, 0);
				PropertySerializer::DeserializeProperty(pMaterialData->GetBufferReference(), typeHash, offset, typeData != nullptr ? typeData->m_Size : 4, node);
			}
			else
			{
				UUID id = node.as<uint64_t>();
				Resource* pResource = AssetManager::GetAssetImmediate(id);
				pMaterialData->AddProperty(name, name, typeHash, pResource);
			}
		}
	}

	void MaterialLoaderModule::ReadPropertyOverrides(YAML::Node& rootNode, MaterialInstanceData* pMaterialData)
	{
		YAML::Node propertiesNode = rootNode["Overrides"];
		if (!propertiesNode.IsSequence()) return;

		size_t resourceCounter = 0;
		for (size_t i = 0; i < propertiesNode.size(); i++)
		{
			YAML::Node propertyNode = propertiesNode[i];
			YAML::Node node;
			std::string name;
			YAML_READ(propertyNode, node, Name, name, std::string);

			size_t propertyIndex = 0;
			if (!pMaterialData->GetPropertyInfoIndex(name, propertyIndex)) continue;
			pMaterialData->m_PropertyOverridesEnable[propertyIndex] = true;

			MaterialPropertyInfo* propertyInfo = pMaterialData->GetPropertyInfoAt(propertyIndex);

			node = propertyNode["Value"];

			if (!propertyInfo->m_IsResource)
			{
				size_t typeHash = propertyInfo->TypeHash();
				size_t offset = propertyInfo->Offset();
				size_t size = propertyInfo->Size();
				PropertySerializer::DeserializeProperty(pMaterialData->GetBufferReference(), typeHash, offset, size, node);
			}
			else
			{
				UUID id = node.as<uint64_t>();
				if(pMaterialData->m_pResources.size() > resourceCounter) pMaterialData->m_pResources[resourceCounter] = AssetManager::GetAssetImmediate(id);
				++resourceCounter;
			}
		}
	}

	void MaterialLoaderModule::WriteShaders(YAML::Emitter& out, MaterialData* pMaterialData)
	{
		out << YAML::Key << "Shaders";
		out << YAML::Value << YAML::BeginMap;
		for (size_t i = 0; i < pMaterialData->ShaderCount(); i++)
		{
			ShaderSourceData* pShaderSourceData = pMaterialData->GetShaderAt(i);
			YAML_WRITE(out, UUID, pShaderSourceData->GetUUID());
			YAML_WRITE(out, Type, pShaderSourceData->GetShaderType());
		}
		out << YAML::EndMap;
	}

	void MaterialLoaderModule::Initialize()
	{

	}

	void MaterialLoaderModule::Cleanup()
	{

	}
}

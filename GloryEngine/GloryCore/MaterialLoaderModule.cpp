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

	MaterialLoaderModule::MaterialLoaderModule() : ResourceLoaderModule("gmat")
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
			pMaterialData->AddProperty(name, name, typeHash, typeData != nullptr ? typeData->m_Size : 4);
			PropertySerializer::DeserializeProperty(pMaterialData->GetBufferReference(), typeHash, offset, typeData != nullptr ? typeData->m_Size : 4, node);
		}
	}

	void MaterialLoaderModule::ReadPropertyOverrides(YAML::Node& rootNode, MaterialInstanceData* pMaterialData)
	{
		YAML::Node propertiesNode = rootNode["Overrides"];
		if (!propertiesNode.IsSequence()) return;
		for (size_t i = 0; i < propertiesNode.size(); i++)
		{
			YAML::Node propertyNode = propertiesNode[i];
			YAML::Node node;
			std::string name;
			YAML_READ(propertyNode, node, Name, name, std::string);

			size_t propertyIndex = 0;
			if (!pMaterialData->GetPropertyInfoIndex(name, propertyIndex)) continue;
			pMaterialData->m_PropertyOverridesEnable[propertyIndex] = true;

			const MaterialPropertyInfo& propertyInfo = pMaterialData->GetPropertyInfoAt(propertyIndex);

			node = propertyNode["Value"];

			size_t typeHash = propertyInfo.TypeHash();
			size_t offset = propertyInfo.Offset();
			size_t size = propertyInfo.Size();
			PropertySerializer::DeserializeProperty(pMaterialData->GetBufferReference(), typeHash, offset, size, node);
		}
	}

	void MaterialLoaderModule::Initialize()
	{

	}

	void MaterialLoaderModule::Cleanup()
	{

	}
}

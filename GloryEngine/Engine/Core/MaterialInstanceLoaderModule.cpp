#include "MaterialInstanceLoaderModule.h"
#include "GLORY_YAML.h"
#include "PropertySerializer.h"
#include "AssetManager.h"
#include "ShaderManager.h"
#include "Engine.h"

#include <fstream>

namespace Glory
{
	MaterialInstanceImportSettings::MaterialInstanceImportSettings() {}
	MaterialInstanceImportSettings::MaterialInstanceImportSettings(const std::string& name)
		: ImportSettings(name) {}

	MaterialInstanceLoaderModule::MaterialInstanceLoaderModule() : ResourceLoaderModule(".gminst")
	{
	}

	MaterialInstanceLoaderModule::~MaterialInstanceLoaderModule()
	{
	}

	const std::type_info& MaterialInstanceLoaderModule::GetModuleType()
	{
		return typeid(MaterialInstanceLoaderModule);
	}

	MaterialInstanceImportSettings MaterialInstanceLoaderModule::ReadImportSettings_Internal(YAML::Node& node)
	{
		return MaterialInstanceImportSettings();
	}

	void MaterialInstanceLoaderModule::WriteImportSettings_Internal(const MaterialInstanceImportSettings& importSettings, YAML::Emitter& out)
	{
	}

	MaterialInstanceData* MaterialInstanceLoaderModule::LoadResource(const std::string& path, const MaterialInstanceImportSettings& importSettings)
	{
		YAML::Node rootNode = YAML::LoadFile(path);
		return LoadMaterialInstanceData(rootNode, importSettings);
	}

	MaterialInstanceData* MaterialInstanceLoaderModule::LoadResource(const void* buffer, size_t length, const MaterialInstanceImportSettings& importSettings)
	{
		YAML::Node rootNode = YAML::Load((const char*)buffer);
		return LoadMaterialInstanceData(rootNode, importSettings);
	}

	void MaterialInstanceLoaderModule::SaveResource(const std::string& path, MaterialInstanceData* pResource)
	{
		MaterialInstanceData* pMaterialInstance = dynamic_cast<MaterialInstanceData*>(pResource);
		YAML::Emitter out;
		out << YAML::BeginMap;
		SaveMaterialInstanceData(pMaterialInstance, out);
		out << YAML::EndMap;
		std::ofstream outStream(path);
		outStream << out.c_str();
		outStream.close();
	}

	MaterialInstanceData* MaterialInstanceLoaderModule::LoadMaterialInstanceData(YAML::Node& rootNode, const MaterialInstanceImportSettings& importSettings)
	{
		YAML::Node node;
		UUID baseMaterial = 0;
		YAML_READ(rootNode, node, BaseMaterial, baseMaterial, uint64_t);
		MaterialData* pMaterialData = m_pEngine->GetAssetManager().GetAssetImmediate<MaterialData>(baseMaterial);
		MaterialInstanceData* pMaterialInstanceData = new MaterialInstanceData(pMaterialData);
		ReadPropertyOverrides(rootNode, pMaterialInstanceData);
		return pMaterialInstanceData;
	}

	void MaterialInstanceLoaderModule::SaveMaterialInstanceData(MaterialInstanceData* pMaterialData, YAML::Emitter& out)
	{
		MaterialData* pBaseMaterial = pMaterialData->m_pBaseMaterial;
		UUID baseMaterial = pBaseMaterial ? pBaseMaterial->m_ID : 0;
		YAML_WRITE(out, BaseMaterial, baseMaterial);

		out << YAML::Key << "Overrides";
		out << YAML::Value << YAML::BeginSeq;
		for (size_t i = 0; i < pMaterialData->PropertyInfoCount(); i++)
		{
			MaterialPropertyInfo* pInfo = pMaterialData->GetPropertyInfoAt(i);
			size_t propertyIndex = 0;
			if (!pMaterialData->GetPropertyInfoIndex(pInfo->m_PropertyDisplayName, propertyIndex)) continue;
			MaterialPropertyInfo* propertyInfo = pMaterialData->GetPropertyInfoAt(propertyIndex);
			if (!pMaterialData->m_PropertyOverridesEnable[i]) continue;


			out << YAML::BeginMap;
			YAML_WRITE(out, DisplayName, pInfo->m_PropertyDisplayName);
			if (!propertyInfo->m_IsResource)
			{
				uint32_t typeHash = propertyInfo->TypeHash();
				size_t offset = propertyInfo->Offset();
				size_t size = propertyInfo->Size();
				m_pEngine->GetSerializers().SerializeProperty("Value", pMaterialData->GetBufferReference(), typeHash, offset, size, out);
			}
			else
			{
				size_t resourceIndex = propertyInfo->Offset();
				size_t index = pMaterialData->GetPropertyIndexFromResourceIndex(resourceIndex);
				const UUID uuid = pMaterialData->GetResourceUUIDPointer(index)->AssetUUID();
				out << YAML::Key << "Value" << YAML::Value << uuid;
			}
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
	}

	void MaterialInstanceLoaderModule::ReadPropertyOverrides(YAML::Node& rootNode, MaterialInstanceData* pMaterialData)
	{
		YAML::Node propertiesNode = rootNode["Overrides"];
		if (!propertiesNode.IsSequence()) return;

		for (size_t i = 0; i < propertiesNode.size(); i++)
		{
			YAML::Node propertyNode = propertiesNode[i];
			YAML::Node node;
			std::string displayName;
			YAML_READ(propertyNode, node, DisplayName, displayName, std::string);

			size_t propertyIndex = 0;
			if (!pMaterialData->GetPropertyInfoIndex(displayName, propertyIndex)) continue;
			pMaterialData->m_PropertyOverridesEnable[propertyIndex] = true;

			MaterialPropertyInfo* propertyInfo = pMaterialData->GetPropertyInfoAt(propertyIndex);

			node = propertyNode["Value"];

			if (!propertyInfo->m_IsResource)
			{
				uint32_t typeHash = propertyInfo->TypeHash();
				size_t offset = propertyInfo->Offset();
				size_t size = propertyInfo->Size();
				m_pEngine->GetSerializers().DeserializeProperty(pMaterialData->GetBufferReference(), typeHash, offset, size, node);
			}
			else
			{
				UUID id = node.as<uint64_t>();
				size_t resourceIndex = propertyInfo->Offset();
				if (pMaterialData->m_Resources.size() > resourceIndex) pMaterialData->m_Resources[resourceIndex] = id;
			}
		}
	}

	void MaterialInstanceLoaderModule::Initialize()
	{

	}

	void MaterialInstanceLoaderModule::Cleanup()
	{

	}
}

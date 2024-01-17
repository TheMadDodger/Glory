#include "MaterialInstanceImporter.h"
#include "EditorApplication.h"

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
		YAML::Node rootNode = YAML::LoadFile(path.string());
		return LoadMaterialInstanceData(rootNode);
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

	MaterialInstanceData* MaterialInstanceImporter::LoadMaterialInstanceData(YAML::Node& rootNode) const
	{
		YAML::Node node;
		UUID baseMaterial = 0;
		YAML_READ(rootNode, node, BaseMaterial, baseMaterial, uint64_t);
		EditorApplication::GetInstance()->GetEngine();
		MaterialData* pMaterialData = EditorApplication::GetInstance()->GetEngine()->GetAssetManager().GetAssetImmediate<MaterialData>(baseMaterial);
		MaterialInstanceData* pMaterialInstanceData = new MaterialInstanceData(pMaterialData);
		ReadPropertyOverrides(rootNode, pMaterialInstanceData);
		return pMaterialInstanceData;
	}

	void MaterialInstanceImporter::SaveMaterialInstanceData(MaterialInstanceData* pMaterialData, YAML::Emitter& out) const
	{
		MaterialData* pBaseMaterial = pMaterialData->GetBaseMaterial();
		UUID baseMaterial = pBaseMaterial ? pBaseMaterial->GetUUID() : 0;
		YAML_WRITE(out, BaseMaterial, baseMaterial);

		out << YAML::Key << "Overrides";
		out << YAML::Value << YAML::BeginSeq;
		for (size_t i = 0; i < pMaterialData->PropertyInfoCount(); ++i)
		{
			MaterialPropertyInfo* pInfo = pMaterialData->GetPropertyInfoAt(i);
			size_t propertyIndex = 0;
			if (!pMaterialData->GetPropertyInfoIndex(pInfo->DisplayName(), propertyIndex)) continue;
			MaterialPropertyInfo* propertyInfo = pMaterialData->GetPropertyInfoAt(propertyIndex);
			if (!pMaterialData->IsPropertyOverriden(i)) continue;

			out << YAML::BeginMap;
			YAML_WRITE(out, DisplayName, pInfo->DisplayName());
			if (!propertyInfo->IsResource())
			{
				uint32_t typeHash = propertyInfo->TypeHash();
				size_t offset = propertyInfo->Offset();
				size_t size = propertyInfo->Size();
				EditorApplication::GetInstance()->GetEngine()->GetSerializers().SerializeProperty("Value", pMaterialData->GetBufferReference(), typeHash, offset, size, out);
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

	void MaterialInstanceImporter::ReadPropertyOverrides(YAML::Node& rootNode, MaterialInstanceData* pMaterialData) const
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
			pMaterialData->EnableProperty(propertyIndex);

			MaterialPropertyInfo* propertyInfo = pMaterialData->GetPropertyInfoAt(propertyIndex);

			node = propertyNode["Value"];

			if (!propertyInfo->IsResource())
			{
				uint32_t typeHash = propertyInfo->TypeHash();
				size_t offset = propertyInfo->Offset();
				size_t size = propertyInfo->Size();
				EditorApplication::GetInstance()->GetEngine()->GetSerializers().DeserializeProperty(pMaterialData->GetBufferReference(), typeHash, offset, size, node);
			}
			else
			{
				UUID id = node.as<uint64_t>();
				size_t resourceIndex = propertyInfo->Offset();
				if (pMaterialData->ResourceCount() > resourceIndex) *pMaterialData->GetResourceUUIDPointer(resourceIndex) = id;
			}
		}
	}

	void MaterialInstanceImporter::Initialize()
	{
	}

	void MaterialInstanceImporter::Cleanup()
	{
	}
}